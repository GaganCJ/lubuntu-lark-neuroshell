#!/usr/bin/env python3
import dbus
import dbus.service
from gi.repository import GLib
import subprocess
import re
import os
import dbus.mainloop.glib # Use the modern, non-deprecated mainloop integration
from jinja2 import Template

# --- AI Backend Configuration ---
# Choose your backend: 'local' (Ollama), 'cloud' (OpenAI), or 'azure' (Azure OpenAI)
AI_BACKEND = os.environ.get("AI_BACKEND", "local")

# --- Model Configuration ---
# Set these via environment variables or change the defaults here.
OLLAMA_MODEL = os.environ.get("OLLAMA_MODEL", "tinydolphin")
OPENAI_MODEL = os.environ.get("OPENAI_MODEL", "gpt-4-turbo-preview")
# For Azure, the model is the deployment name, configured by AZURE_OPENAI_DEPLOYMENT

# --- Import appropriate AI library ---
if AI_BACKEND == "cloud":
    import openai
elif AI_BACKEND == "azure":
    from openai import AzureOpenAI
else:
    import ollama

BANNED_PATTERNS = [r"rm\s+-rf\s+/", r"mkfs", r"dd\s+if=", r"&gt; /dev/sda", r"chmod\s+-R\s+777\s+/"]

# The Copilot styling theme is now compiled server-side inside this HTML Jinja template block
HTML_RESPONSE_TEMPLATE = """
&lt;br&gt;
&lt;b style="color: #58a6ff;"&gt;🤖 NeuroShell:&lt;/b&gt;
&lt;div&gt;{{ reply }}&lt;/div&gt;

{% if bash_command %}
&lt;div style="margin-top: 8px; margin-bottom: 4px; font-size: 11px; color: #8b949e; font-family: monospace;"&gt;
    Executed Terminal Action:
&lt;/div&gt;
&lt;pre style="background-color: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 10px; color: #ff7b72; font-family: monospace;"&gt;{{ bash_command }}&lt;/pre&gt;
{% endif %}

{% if output %}
&lt;div style="margin-top: 4px; font-size: 11px; color: #8b949e; font-family: monospace;"&gt;
    System Standard Output:
&lt;/div&gt;
&lt;pre style="background-color: #070a0e; border: 1px solid #21262d; border-radius: 6px; padding: 10px; color: #39ff14; font-family: monospace;"&gt;{{ output }}&lt;/pre&gt;
{% endif %}
&lt;hr style="border: 0; border-top: 1px solid #21262d; margin-top: 12px;"&gt;
"""

def safety_check(cmd_string):
    for pattern in BANNED_PATTERNS:
        if re.search(pattern, cmd_string, re.IGNORECASE):
            return False
    return True

def extract_tag_content(text, tag_name):
    pattern = f"&lt;{tag_name}&gt;(.*?)&lt;/{tag_name}&gt;"
    match = re.search(pattern, text, re.DOTALL)
    return match.group(1).strip() if match else ""

class NeuroShellLocalAutonomousService(dbus.service.Object):
    def __init__(self):
        bus_name = dbus.service.BusName('org.lxqt.neuroshell', bus=dbus.SessionBus())
        dbus.service.Object.__init__(self, bus_name, '/org/lxqt/neuroshell')
        self.template = Template(HTML_RESPONSE_TEMPLATE)
        print(f"[Lark Engine] Control Plane Engaged. Backend: {AI_BACKEND}, Model: {self._get_model_in_use()}")

    def _get_model_in_use(self):
        if AI_BACKEND == "cloud":
            return OPENAI_MODEL
        elif AI_BACKEND == "azure":
            return os.environ.get("AZURE_OPENAI_DEPLOYMENT", "azure-deployment")
        else: # local
            return OLLAMA_MODEL

    @dbus.service.method(dbus_interface='org.lxqt.neuroshell.Interface', in_signature='', out_signature='s')
    def GetModelInUse(self):
        """Returns the name of the AI model currently in use."""
        return self._get_model_in_use()

    @dbus.service.method(dbus_interface='org.lxqt.neuroshell.Interface', in_signature='s', out_signature='s')
    def ProcessIntent(self, raw_user_payload):
        system_prompt = f"""
        You are the autonomous execution engine for a Lubuntu Linux desktop environment.
        User Intent: "{raw_user_payload}"
        Construct the exact bash commands needed to complete the task.
        You must respond following this exact block format template:

        &lt;THOUGHT&gt;Explain what you are trying to achieve briefly&lt;/THOUGHT&gt;
        &lt;PRIVILEGED&gt;true or false&lt;/PRIVILEGED&gt;
        &lt;BASH&gt;The exact terminal commands to execute&lt;/BASH&gt;
        &lt;REPLY&gt;A conversational explanation for the user&lt;/REPLY&gt;
        """
        try:
            llm_text = ""
            if AI_BACKEND == "cloud":
                if not os.environ.get("OPENAI_API_KEY"):
                    raise Exception("OPENAI_API_KEY environment variable not set.")

                client = openai.OpenAI()
                response = client.chat.completions.create(
                    model=OPENAI_MODEL,
                    messages=[
                        {"role": "system", "content": "You are the autonomous execution engine for a Lubuntu Linux desktop environment. You must follow the user's instructions and respond in the specified XML format."},
                        {"role": "user", "content": system_prompt}
                    ],
                    temperature=0.1,
                )
                llm_text = response.choices[0].message.content.strip()
            elif AI_BACKEND == "azure":
                if not all(os.environ.get(k) for k in ["AZURE_OPENAI_ENDPOINT", "AZURE_OPENAI_KEY", "AZURE_OPENAI_DEPLOYMENT"]):
                    raise Exception("Azure environment variables (ENDPOINT, KEY, DEPLOYMENT) not set.")

                client = AzureOpenAI(
                    azure_endpoint=os.environ["AZURE_OPENAI_ENDPOINT"],
                    api_key=os.environ["AZURE_OPENAI_KEY"],
                    api_version="2024-02-15-preview"
                )
                response = client.chat.completions.create(
                    model=os.environ["AZURE_OPENAI_DEPLOYMENT"],
                    messages=[
                        {"role": "system", "content": "You are the autonomous execution engine for a Lubuntu Linux desktop environment. You must follow the user's instructions and respond in the specified XML format."},
                        {"role": "user", "content": system_prompt}
                    ],
                    temperature=0.1,
                )
                llm_text = response.choices[0].message.content.strip()
            else:  # local backend
                response = ollama.generate(model=OLLAMA_MODEL, prompt=system_prompt, options={"temperature": 0.1})
                llm_text = response['response'].strip()

            privileged = extract_tag_content(llm_text, "PRIVILEGED").lower() == "true"
            bash_cmd = extract_tag_content(llm_text, "BASH")
            reply = extract_tag_content(llm_text, "REPLY")

            output_data = ""
            if bash_cmd:
                if not safety_check(bash_cmd):
                    return "&lt;b style='color:#f85149;'&gt;Security Exception:&lt;/b&gt; Dangerous bash string blocked."
                
                exec_args = ["lxqt-sudo", "--", "bash", "-c", bash_cmd] if privileged else ["bash", "-c", bash_cmd]
                result = subprocess.run(exec_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=30)
                output_data = result.stdout.strip() if result.returncode == 0 else result.stderr.strip()

            # Render the data fields using Jinja2 into a flat, static HTML block
            rendered_html = self.template.render(
                reply=reply,
                bash_command=bash_cmd,
                output=output_data
            )
            return rendered_html

        except Exception as e:
            return f"&lt;b style='color:#f85149;'&gt;NeuroShell Pipeline Error:&lt;/b&gt; {str(e)}"

if __name__ == '__main__':
    # Set up the D-Bus main loop to integrate with GLib's event loop
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    service = NeuroShellLocalAutonomousService()
    GLib.MainLoop().run()
