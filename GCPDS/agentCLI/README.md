# Agents CLI — Guide for collaborators

This repository hosts an Agents CLI for coordinating multiple assistant agents (role-based agents, chatbots and tool connectors). This README provides concepts, rationale, installation steps (PowerShell + WSL), usage examples (Copilot, Claude, GPT-family, Gemini), best prompt practices, and guidance for collaborative environments (Docker, WSL, conda/venv, C toolchains).

## Concepts & Why this tool is useful

- Multi-agent coordination: assign distinct roles (Coordinator, Interpreter, Audio/Processing, Backend) to get consistent, context-aware outputs.
- Role, Objective, Context, Reasoning: prompts should declare these to reduce hallucinations and increase repeatability (see basisReadme.md).
- Use-cases: research experimentation, prototype orchestration, quality-assurance workflows (see ROLE-BASED_GUIDE.md for role-specific paths).

Supported example agent backends (examples):
- GitHub Copilot (developer assistant)
- Anthropic Claude (instruction-following)
- OpenAI GPT-4 / GPT-5-mini (creative & reasoning)
- Google Gemini (multimodal tasks)

## Quick start — Clone the repo

Windows PowerShell (recommended):

1. Open PowerShell (as user):
   git clone https://github.com/Macreat/PersonalStuff.git
   cd PersonalStuff\GCPDS\agentCLI

2. Inspect docs:
   - basisReadme.md  — conceptual & system design
   - ANE2/maintenance/qualityAssurance/ROLE-BASED_GUIDE.md — role-specific workflows

## Install (two common flows)

A. If the repo uses Python components (recommended for reproducibility):

PowerShell:
- python -m venv .venv
- .\.venv\Scripts\Activate.ps1
- if (Test-Path requirements.txt) { pip install -r requirements.txt }

WSL / Linux:
- sudo apt update && sudo apt install -y python3-venv python3-pip
- git clone <repo>
- python3 -m venv .venv
- source .venv/bin/activate
- pip install -r requirements.txt

B. If the repo uses Node.js (check package.json):

PowerShell:
- npm install

WSL:
- npm install

Notes:
- Use the appropriate package manager depending on the repo: pip for Python, npm for Node.
- For reproducible environments prefer Docker (see below).

## Running the Agents CLI (examples)

This section follows GitHub Copilot CLI patterns for terminal-driven agent workflows. See the official docs for installation details: https://docs.github.com/en/copilot/concepts/agents/copilot-cli/about-copilot-cli

Supported OS: Windows (PowerShell, WSL), Linux, macOS.

Interactive mode (recommended for exploration):
- Start an interactive session: `copilot`
- Two interactive modes: ask/execute (default) and plan mode. Press Shift+Tab to switch to plan mode for structured, multi-step plans.
- While running, you can steer the conversation, enqueue follow-ups, and give inline feedback when Copilot requests tool access.

Programmatic mode (one-shot prompts):
- Run a single prompt and exit: `copilot -p "Show me this week's commits and summarize them"`
- Allow specific tools for programmatic runs: `copilot -p "..." --allow-tool='shell(git)'`
- To permit all tools for the session (risky), use: `--allow-all-tools` (use with extreme caution).

Examples (PowerShell / WSL):
- Interactive: `copilot`
- Programmatic with git access: `copilot -p "List modified Python files" --allow-tool='shell(git)'`
- Programmatic full automation (dangerous): `copilot -p "Run tests and create a PR if they pass" --allow-all-tools`

Security & trusted directories:
- Launch Copilot CLI from a directory you trust. Copilot will ask to confirm trust for the current directory and its subdirectories.
- When Copilot requests permission to use a tool, you typically choose: 1) Yes, 2) Yes (for session), 3) No (and provide feedback). Review each request carefully.

Context management & helpful commands:
- `/compact` — compress conversation history when approaching token limits.
- `/context` — show token usage breakdown and what is in context.

Customization & integrations:
- Use custom instructions, MCP servers, hooks, and skills to connect Copilot CLI to your project-specific data sources and workflows.
- Configure secrets and API keys in environment variables or a secrets manager; do not commit them to the repo.

How this maps to Agents CLI in this repo:
- Use `copilot` for exploratory, interactive role-based sessions (e.g., "Coordinator", "Interpreter").
- Use programmatic `copilot -p` calls in CI or automation scripts when you want deterministic one-shot tasks (remember to scope tool approvals).

See the Copilot CLI docs for full install instructions and advanced configuration: https://docs.github.com/en/copilot/concepts/agents/copilot-cli/about-copilot-cli

## Prompting best practices (use this as a template)

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

This structured approach matches the ROLE → OBJECTIVE → CONTEXT → REASONING pattern in basisReadme.md.

## Exploring / Examples to try

- Run a role-based review pass: assign "Reviewer" role and ask for TODOs per module.
- Use an "Interpreter" + "Backend agent" pair: Interpreter extracts entities, Backend persists to a local JSON DB.
- Multimodal sample: audio → Audio agent computes spectrogram → Interpreter summarizes findings.

Refer to ROLE-BASED_GUIDE.md for specific role entry points and code module maps for wideband/narrowband/voice services.

## Collaborative & Reproducible environments

1) Docker (recommended for consistent CI/dev):
- Create Dockerfile that includes required runtimes (Python, build tools for C, ffmpeg if multimodal).
- Example run: docker build -t agentcli:latest . && docker run --rm -it -v "%CD%":/workspace agentcli:latest

2) WSL + VSCode Remote
- Use VSCode Remote - WSL extension for consistent Linux tools on Windows.
- Use Git + Remote development to keep builds identical to CI.

3) Python envs: conda or venv
- For heavier data or ML use conda envs; for lightweight reproducibility use venv + requirements.txt.

4) C development & toolchains
- For C components, prefer building inside WSL or Docker (install build-essential / clang / msys2 on Windows).
- Use consistent toolchains in CI to avoid "works on my machine" issues.

## Integrating other tools (text, image, video)

- Text: editors (VSCode), linters, unit tests. Use Copilot extensions for developer productivity.
- Images: ImageMagick or Pillow for image ops; OpenAI or Gemini image endpoints for multimodal prompts.
- Video/audio: ffmpeg, librosa, PyTorch/TensorFlow for deep analysis; Audio agent design is in basisReadme.md.

## Security & Open Source

- This project favors open-source tooling; keep API keys and secrets out of the repo (.gitignore .env files).
- Document data provenance and citation when using proprietary models.

## How to contribute (brief)

1. Pick a role/task from ROLE-BASED_GUIDE.md
2. Create a branch: feature/<role>-<short-desc>
3. Add tests, type hints, and docs (follow Google docstring style)
4. Run tests and linters before PR
5. Reference the role in the PR description

## References
- basisReadme.md — conceptual & structural guidance
- ANE2/maintenance/qualityAssurance/ROLE-BASED_GUIDE.md — role-specific workflows and module maps

---

If you want, I can add runnable examples (Dockerfile, example runner) or tailor the install instructions to the repo's actual stack — tell me if this repo uses Python, Node, or a specific build system and I’ll update README with exact commands.
