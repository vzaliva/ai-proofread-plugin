# Evolution AI Proofread Plugin

This plugin adds a "AI Proofread" button to the message composition
toolbar of the Evolution email client. The button allows you to
proofread the email text using OpenAI's GPT API before sending it.

## Building

```
$ mkdir _build && cd _build
$ cmake -DCMAKE_INSTALL_PREFIX=~/.local/share/evolution/modules \
           -DFORCE_INSTALL_PREFIX=ON ..
$ make && make install
```

## Development

To use under vscode first generate compile_commands.json:

```
$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .
```

## License

This plugin is open-source and licensed under the MIT License.

## Author

Vadim Zaliva, 2024

