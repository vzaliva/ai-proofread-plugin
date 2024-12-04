import re
import requests
import os
import gi

gi.require_version('Gtk', '3.0')
gi.require_version('Peas', '1.0')
gi.require_version('GtkSource', '4')
from gi.repository import GObject, Gtk, Peas, GtkSource

# Define the path to the authinfo file
authinfo_path = os.path.expanduser("~/.authinfo")

class ProofreadPlugin(GObject.Object, Peas.Activatable):
    __gtype_name__ = 'ProofreadPlugin'
    object = GObject.Property(type=GObject.Object)

    def __init__(self):
        super(ProofreadPlugin, self).__init__()

    def do_activate(self):
        # Get the message composition window
        window = self.object.get_toplevel()
        if window:
            # Add a proofread button to the toolbar
            toolbar = window.get_toolbar()
            if toolbar:
                self.proofread_button = Gtk.ToolButton(label="AI Proofread")
                self.proofread_button.connect("clicked", self.on_proofread_button_clicked)
                toolbar.insert(self.proofread_button, -1)
                toolbar.show_all()

    def do_deactivate(self):
        # Remove the proofread button when deactivating the plugin
        if hasattr(self, 'proofread_button'):
            self.proofread_button.destroy()

    def on_proofread_button_clicked(self, button):
        # Get the currently edited message text
        message_text = self.get_current_message_text()
        if message_text:
            api_key = self.load_api_key_from_authinfo()
            if api_key:
                proofread_text = self.proofread_with_gpt(message_text, api_key)
                if proofread_text:
                    self.set_current_message_text(proofread_text)

    def load_api_key_from_authinfo(self):
        try:
            with open(authinfo_path, 'r') as authfile:
                content = authfile.read()
                match = re.search(r'machine api\.openai\.com login apikey password (\S+)', content)
                if match:
                    return match.group(1)
        except FileNotFoundError:
            print("Authinfo file not found.")
        except Exception as e:
            print(f"Error reading authinfo file: {e}")
        return None

    def proofread_with_gpt(self, text, api_key):
        if not api_key:
            print("API key not found.")
            return None

        headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}"
        }
        data = {
            "model": "gpt-3.5-turbo",
            "messages": [
                {"role": "user", "content": f"Please proofread the following text: {text}"}
            ],
            "max_tokens": 1024
        }
        try:
            response = requests.post("https://api.openai.com/v1/chat/completions", json=data, headers=headers)
            response.raise_for_status()
            result = response.json()
            return result["choices"][0]["message"]["content"]
        except requests.exceptions.RequestException as e:
            print(f"Error communicating with OpenAI API: {e}")
            return None

    def get_current_message_text(self):
        # Assuming there is a text view for editing
        editor = self.object.get_child()
        if isinstance(editor, GtkSource.View):
            buffer = editor.get_buffer()
            start, end = buffer.get_bounds()
            return buffer.get_text(start, end, True)
        return None

    def set_current_message_text(self, new_text):
        # Assuming there is a text view for editing
        editor = self.object.get_child()
        if isinstance(editor, GtkSource.View):
            buffer = editor.get_buffer()
            buffer.set_text(new_text)
