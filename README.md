# MistralPuter for Cardputer ADV

This is the previously adapted CardputerGPT-based Mistral AI client, packaged as a PlatformIO project with GitHub Actions continuous integration. It uses no compile-time API keys.

## Build on GitHub (no PC compiler required)

1. Create a new GitHub repository (suggested name `MistralPuter`).
2. Add **all the contents of this folder** to the repository root, preserving `.github/workflows/build-firmware.yml` and `src/*` paths. Do not upload just the ZIP file.
3. GitHub Actions will run on the initial push to `main`. Alternatively, select **Actions > Build Cardputer ADV firmware > Run workflow**.
4. Open the successful build, and download the `MistralPuter-ADV-firmware` artifact (a ZIP containing `MistralPuter-ADV.bin`).
5. Extract the `.bin` and use M5Launcher WebUI to upload and launch it. Back up your existing firmware/configuration first.

A successful compilation does NOT establish device functionality; hardware, launcher partitions, Wi-Fi, keyboard mapping and live TLS certificate chain must be checked on the actual device. The `.bin` is an application firmware, not a merged full-flash image.

## Implementation

* On-device Wi-Fi and API key entry; stored in unencrypted NVS Preferences.
* Mistral chat completion API: `https://api.mistral.ai/v1/chat/completions`.
* Default model: `mistral-small-latest`.
* Commands: `:wifi`, `:key`, `:model`, `:new`, `:help`, `:erase`.
* Limited response length (512 output tokens) and short rolling history.
* GitHub build uses PlatformIO `espressif32@6.7.0`, M5Cardputer and ArduinoJson 7.
* Licensed GPL-3.0, retaining upstream copyright/licence.

## Troubleshooting

If the GitHub Action fails, read the failed **Build firmware** step logs. Package versions or code may require updating. Do not install a file from a failed or incomplete build.

The current certificate bundle is inherited from the modified source and has not been verified against the live API endpoint. Do not bypass TLS verification; update the included trusted roots if the Mistral TLS connection fails.

Based on https://github.com/shaahin/CardputerGPT. This release has not been tested or compiled locally.
