---
name: magisk-git
description: Guidelines and instructions for Git operations, commit policies, and commit message formatting in the Magisk repository. Use when committing changes, amending commits, or formatting commit messages.
---

# Magisk Git & Commit Guidelines

Guidelines and rules for Git operations and commit management in the Magisk repository.

## 1. Commit Control & Approval

- **NEVER** commit changes or amend an existing git commit without the user's explicit request or approval.
- Do not stage or commit files automatically as part of automated steps or task conclusions unless explicitly instructed by the user.

## 2. Commit Message Formatting

When explicitly requested to commit changes, format commit messages according to the following rules:

1. **50/72 Rule:**
   - **Subject line:** At most 50 characters. Keep it concise, descriptive, and written in the imperative mood (e.g., `Fix build failure on Android 14`, `Add support for custom mounts`).
   - **Blank line:** Always include a blank line between the subject line and the commit body.
   - **Body text:** Wrap body lines at 72 characters maximum. Explain the *what* and *why* behind the change.

2. **Assisted-by Trailer:**
   - Include an `Assisted-by: <Friendly Name of Current Model>` trailer in the commit message body (e.g., `Assisted-by: Gemini 3.7 Flash`, `Assisted-by: GPT-4o`, `Assisted-by: Claude 3.5 Sonnet`).
   - **Note:** Always specify the underlying LLM model name, NOT the agent framework, runner, or harness name (such as Antigravity, AGY, etc.).

## 3. Example Commit Message

```text
Short summary of changes in 50 characters or less

Detailed explanation of why this change was made and what problems
it solves. Wrap all lines in the message body to at most 72
characters.

Assisted-by: <Friendly Name of Current Model>
```
