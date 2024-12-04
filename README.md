# Evolution AI Proofread Plugin

This plugin adds a "AI Proofread" button to the message composition
toolbar of the Evolution email client. The button allows you to
proofread the email text using OpenAI's GPT API before sending it.

## Features
- Adds a "AI Proofread" button to the Evolution email composition toolbar.
- Utilizes OpenAI's GPT model to proofread the content of the email.
- Loads API key from a `~/.authinfo` file to securely access OpenAI's services.

## Prerequisites
- Evolution email client installed (version 3.0 or later).
- Python 3.
- The following packages:
  - `python3-gi`
  - `python3-requests`
  - `gir1.2-gtk-3.0`
  - `gir1.2-peas-1.0`
  - `gir1.2-gtksource-4`

## Installation

1. **Install Dependencies**
   
   First, ensure that all required dependencies are installed. Run the following command:
   ```bash
   sudo apt update
   sudo apt install evolution python3 python3-gi python3-requests gir1.2-gtk-3.0 gir1.2-peas-1.0 gir1.2-gtksource-4
   ```

2. **Create Plugin Directory**
   
   Create a directory for the plugin:
   ```bash
   mkdir -p ~/.local/share/evolution/plugins/ai-proofread-plugin
   ```

3. **Copy Plugin Script files**
   
   Copy files into the newly created directory:
   ```bash
   cp ai-proofread-plugin.plugin ai_proofread_plugin.py ~/.local/share/evolution/plugins/ai-proofread-plugin/
   ```
   Paste the content of the plugin script into this file and save it.

4. **Configure API Key**

   Make sure you have your OpenAI API key saved in `~/.authinfo`. The `.authinfo` file should contain the following entry:
   ```
   machine api.openai.com login apikey password <your_openai_api_key>
   ```
   Replace `<your_openai_api_key>` with your actual OpenAI API key. Ensure that the file permissions are set to be readable only by you:
   ```bash
   chmod 600 ~/.authinfo
   ```

5. **Reload Evolution Plugins**

   Restart Evolution to load the new plugin:
   ```bash
   evolution --force-shutdown
   evolution &
   ```

6. **Enable the Plugin**
   
   Open Evolution, navigate to **Edit** → **Plugins**, and enable the **AI Proofread Plugin**.

## Usage

- Compose a new email in Evolution.
- Click on the "AI Proofread" button in the message composition toolbar to have the email content proofread by OpenAI's GPT model.

## Troubleshooting

- **Plugin Not Showing Up**: Ensure that all dependencies are installed and that Evolution is restarted after installation.
- **Errors with API Key**: Ensure that the `~/.authinfo` file is correctly formatted and readable only by your user.
- **Permission Issues**: Check file permissions for both the `.authinfo` file and the plugin files.

## License

This plugin is open-source and licensed under the MIT License.

## Author

Vadim Zaliva, 2024

