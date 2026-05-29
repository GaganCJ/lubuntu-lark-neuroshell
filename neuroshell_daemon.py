#!/usr/bin/env python3
import dbus
import dbus.service
from gi.repository import GLib
import dbus.mainloop.glib # Use the modern, non-deprecated mainloop integration
import subprocess
import os
import re
import ollama
from jinja2 import Environment, select_autoescape

BANNED_PATTERNS = [r"rm\s+-rf\s+/", r"mkfs", r"dd\s+if=", r"> /dev/sda", r"chmod\s+-R\s+777\s+/"]

def safety_check(cmd_string):
    for pattern in BANNED_PATTERNS:
        if re.search(pattern, cmd_string, re.IGNORECASE):
            return False
    return True

def extract_tag_content(text, tag_name):
    pattern = f"<{tag_name}>(.*?)</{tag_name}>"
    match = re.search(pattern, text, re.DOTALL)
    return match.group(1).strip() if match else ""

class NeuroShellLocalAutonomousService(dbus.service.Object):
    def __init__(self):
        bus_name = dbus.service.BusName('org.lxqt.neuroshell', bus=dbus.SessionBus())
        dbus.service.Object.__init__(self, bus_name, '/org/lxqt/neuroshell')
        print("[Lark Engine] Local Autonomous Control Plane Engaged.")

    @dbus.service.method(dbus_interface='org.lxqt.neuroshell.Interface', in_signature='s', out_signature='s')
    def ProcessIntent(self, raw_user_payload):
        system_prompt = f"""
        You are the autonomous execution engine for a Lubuntu Linux desktop environment.
        User Intent: "{raw_user_payload}"
        Construct the exact bash commands needed to complete the task.
        You must respond following this exact block format template:

        <THOUGHT>Explain what you are trying to achieve briefly</THOUGHT>
        <PRIVILEGED>true or false</PRIVILEGED>
        <BASH>The exact terminal commands to execute</BASH>
        <REPLY>A conversational explanation for the user</REPLY>
        """
        try:
            response = ollama.generate(model='gemma3:1b-it-qat', prompt=system_prompt, options={"temperature": 0.1})
            llm_text = response['response'].strip()

            privileged = extract_tag_content(llm_text, "PRIVILEGED").lower() == "true"
            bash_cmd = extract_tag_content(llm_text, "BASH")
            reply = extract_tag_content(llm_text, "REPLY")

            if bash_cmd:
                if not safety_check(bash_cmd):
                    return "<h3>Security Exception</h3>Unverified or dangerous bash sequence blocked."

                exec_args = ["lxqt-sudo", "bash", "-c", bash_cmd] if privileged else ["bash", "-c", bash_cmd]
                result = subprocess.run(exec_args, capture_output=True, text=True, timeout=30)
                
                output = result.stdout.strip() if result.returncode == 0 else result.stderr.strip()
                
                # Use Jinja2 for safe HTML rendering
                env = Environment(autoescape=select_autoescape(['html']))
                template_str = "{{ reply_text }}<br><br><b>System Output:</b><pre>{{ shell_output }}</pre>"
                template = env.from_string(template_str)
                return template.render(reply_text=reply, shell_output=output)

            return reply if reply else "Parsing block format bounds failed."
        except Exception as e:
            return f"NeuroShell Local Pipeline Error: {str(e)}"

if __name__ == '__main__':
    # Set up the D-Bus main loop to integrate with GLib's event loop
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    service = NeuroShellLocalAutonomousService()
    GLib.MainLoop().run()