# Agents CLI — Guide for co-workers

This repository hosts an Agents CLI for coordinating multiple assistant agents (role-based agents, chatbots and tool connectors), in order to explain the power of an AI agent directly into our terminal, enabling coding, automation, and workflow management.

 _This README provides concepts, rationale, installation steps (PowerShell + WSL), usage examples (Copilot, Claude, GPT-family, Gemini), best prompt practices, and guidance for collaborative environments (Docker, WSL, conda/venv, C toolchains)._

## Concepts & Why this tool is useful

Makes building, editing and deploying AI agents on a simple way. 

- Multi-agent coordination: assign distinct roles (Coordinator, Interpreter, Audio/Processing, Backend) to get consistent, context-aware outputs.
- Role, Objective, Context, Reasoning: prompts should declare these to reduce hallucinations and increase repeatability (see basisReadme.md).
- Use-cases: research experimentation, prototype orchestration, quality-assurance workflows (see ROLE-BASED_GUIDE.md for role-specific paths).

Supported example agent backends (examples):
- GitHub Copilot (developer assistant)
- Anthropic Claude (instruction-following)
- OpenAI GPT-4 / GPT-5-mini (creative & reasoning)
- gemini CLI  (multimodal tasks)


## Copilot CLI — installation, get started, and concepts

This section pulls together official Copilot documentation so contributors can install and run Copilot CLI from the terminal, understand core concepts, and find tutorials.

Installation (choose one):

- npm (cross-platform, Node.js 22+ required):
  - npm install -g @github/copilot
  - (If npm ignores scripts) npm_config_ignore_scripts=false npm install -g @github/copilot

- WinGet (Windows):
  - winget install GitHub.Copilot

- Homebrew (macOS / Linux):
  - brew install copilot-cli

- Install script (macOS / Linux):
  - curl -fsSL https://gh.io/copilot-install | bash

- Or download a release binary from: https://github.com/github/copilot-cli/releases

Authentication:

- On first run, launch `copilot` and use the `/login` command to authenticate via the browser.
- Or export a fine-grained personal access token with "Copilot Requests" permission:
  - export COPILOT_GITHUB_TOKEN="ghp_..."

Starting Copilot and basic usage:

- Interactive (conversational):
  - Run `copilot` in the terminal (it uses the current working directory as the context). Copilot will ask you to confirm that the directory is trusted before reading/modifying files.
  - Interactive modes: ask/execute (default) and plan mode — press Shift+Tab to enter plan mode for structured multi-step plans.

- Programmatic (one-shot):
  - `copilot -p "List TODOs in src/ and create issues"`
  - Allow tools for automation: add `--allow-tool='shell(git)'` or `--allow-all-tools` (dangerous; prefer scoping to specific tools).

Security notes:

- Copilot runs with the privileges of the user. Only run it from trusted directories and review any requested tool permissions.
- Copilot prompts to allow tools (Yes / Yes for session / No). Choose conservatively.

Core concepts & where to learn more:

- What is Copilot: high-level AI assistant integrated into editors, GitHub, and CLI. Useful for code completion, code review, repo tasks, and automation.
- Modes: inline suggestions (editor), chat (IDE & GitHub), and CLI (interactive/programmatic).
- Customization: custom instructions, MCP servers (data connectors), hooks, and skills to adapt Copilot to project workflows.

Useful commands & tips:

- `/compact` — compress conversation history when token budgets are high.
- `/context` — show what is currently in Copilot's context window.
- Use `.copilot/custom-instructions` or repo-level custom instruction files to give Copilot project-specific guidance.

References, how-tos and tutorials

- Official Copilot docs home: https://docs.github.com/en/copilot
- Copilot CLI: https://docs.github.com/en/copilot/how-tos/set-up/install-copilot-cli
- Using Copilot CLI: https://docs.github.com/en/copilot/concepts/agents/copilot-cli/about-copilot-cli
- Get started / tutorials: https://docs.github.com/en/copilot/getting-started-with-github-copilot

How this ties into this Agents CLI repo

- Use Copilot CLI for exploratory, role-based sessions in this repository. Run `copilot` from a module directory (e.g., `GCPDS/agentCLI/`) so Copilot can read nearby files and provide targeted suggestions.
- For CI automation, use `copilot -p` in scripts, but carefully scope `--allow-tool` flags.

If you want, add a small shell snippet or GitHub Actions job that installs Copilot and runs a one-shot prompt in CI — say if you want to automate lightweight repo checks.

## Gemini CLI — installation, concepts, and quickstart

Gemini CLI brings Google Gemini models into the terminal with support for skills, sandboxing, headless automation, and project-aware context.

Installation (recommended):

- npm (cross-platform):
  - npm install -g @google/gemini-cli

- Or download binaries / releases at: https://github.com/google/gemini/gemini-cli/releases

Authentication and first run:

- After installing, run `gemini` to start an interactive session. Follow on-screen prompts to authenticate (browser flow) or consult the docs for enterprise configuration.
- For CI/headless usage consult the Gemini docs for authentication and token approaches.

Quick usage examples:

- Interactive session: `gemini` (starts a conversational agent using current working directory as context).
- Headless/one-shot (example): `gemini -p "List TODOs in src/ and summarize"`
- Explore skills and tools: use the built-in help and `--help` to list commands and skill tutorials.

Key concepts:

- Skills: specialized agent modules that provide domain expertise (e.g., file management, shell execution, web fetch).
- Sandbox & trusted folders: Gemini CLI isolates tool execution and asks for folder trust before using powerful tools.
- Checkpointing & sessions: automatic snapshots and session history allow rewinding conversations.
- Headless mode: run programmatic prompts for CI automation and scripts.

Security notes:

- Only run Gemini in trusted directories and review any requested permissions for executing shell tools.
- Use sandboxing features and forbiddenPaths to constrain tool capabilities in sensitive environments.

References & tutorials:

- Official Gemini CLI docs: https://use-gemini.com/docs#installation
- Tutorials: file management, skills, memory and context, sandboxing, and MCP server setup (see docs index).

How this ties into this Agents CLI repo

- Use `gemini` for multimodal and skill-driven local workflows. Run it from module directories (e.g., `GCPDS/agentCLI/`) so Gemini can access nearby files and provide context-aware help.
- Prefer interactive mode for exploration and headless mode for scripted automation in CI (with careful auth and scoped permissions).

## Prompting best practices

Start prompts with:
1. Role: (who should respond — e.g., "Coordinator agent")
2. Objective: (what to accomplish)
3. Context: (relevant repo/files, constraints)
4. Output format: (bullet list, JSON, code block)
5. Example: provide a small example of expected output

Example prompt:
- Role: "Code reviewer (concise)"
- Objective: "Find missing unit tests for src/processing"
- Context: "Repo path: src/processing; test framework: pytest"
- Output: JSON list of files & recommended tests


ROLE → OBJECTIVE → CONTEXT → REASONING pattern.



## Exploring / Examples to try

- Run a role-based review pass: assign "Reviewer" role and ask for TODOs per module.
- Use an "Interpreter" + "Backend agent" pair: Interpreter extracts entities, Backend persists to a local JSON DB.
- Multimodal sample: audio → Audio agent computes spectrogram → Interpreter summarizes findings.



_go to_  [`well prompt information`](wellPrompt/README.md) _for more detail_ 


## Collaborative & Reproducible environments

1) Docker (recommended for consistent CI/dev):
- Create Dockerfile that includes required runtimes (Python, build tools for C, ffmpeg if multimodal).
- Example run: docker build -t agentcli:latest . && docker run --rm -it -v "%CD%":/workspace agentcli:latest

2) WSL + VSCode Remote
- Use VSCode Remote - WSL extension for consistent Linux tools on Windows.
- Use Git + Remote development to keep builds identical to CI.

3) Python envs: conda or venv
- For heavier data or ML use conda envs; for lightweight reproducibility use venv/conda + requirements.txt.

4) C development & toolchains
- For C components, prefer building inside WSL or Docker (install build-essential / clang / msys2 on Windows).
- Use consistent toolchains in CI to avoid "works on my machine" issues.

_go to_  [`collaboratory C environment guide`](C-Collab-Env-Guide.md) _for more detail_ 



## Integrating other tools (text, image, video)

- Text: editors (VSCode), linters, unit tests. Use Copilot extensions for developer productivity.
- Images: ImageMagick or Pillow for image ops; OpenAI or Gemini image endpoints for multimodal prompts.
- Video/audio: ffmpeg, librosa, PyTorch/TensorFlow for deep analysis; Audio agent design is in basisReadme.md.

## Security & Open Source

- This project favors open-source tooling; keep API keys and secrets out of the repo (.gitignore .env files).
- Document data provenance and citation when using proprietary models.
- We can create an organization and work on anthropic using CLAUDE. 



## How to contribute (brief)

refers to [`deployment guide`](../ANE2/maintenance/qualityAssurance/docs/reference/deploymentGuide.md)
for more details 

A. If the repo uses Python components (recommended for reproducibility): 
Clone as this: 
- git clone <https://github.com/Macreat/GCPDS-CW.git>

then, depending on the OS : 

PowerShell:
- python -m venv .venv _(also can use conda)_
- .\.venv\Scripts\Activate.ps1
- if (Test-Path requirements.txt) { pip install -r requirements.txt }

WSL / Linux:
- sudo apt update && sudo apt install -y python3-venv python3-pip
- git clone <repo>
- python3 -m venv .venv
- source .venv/bin/activate
- pip install -r requirements.txt
  

After completing these steps, we can align and establish a collaborative environment: 

1. Pick a role/task from [`collaboratory environment guide`](../ANE2/maintenance/qualityAssurance/ROLE-BASED_GUIDE.md) for specific role entry points and code module maps for wideband/narrowband/voice services.
2. Create a branch (after confirm the repo cloned): <feature>-<role>-<short-desc>
3. Add tests, type hints, and docs (follow Google docstring style)
4. Run tests and linters before PR and/or commit.
5. Reference the role in the PR/commit description (optional)


