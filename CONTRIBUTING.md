# Contributing to Hercules

Hello! Third party patches are essential to keep Hercules great. We want to
keep it as easy as possible to contribute changes that get things working in
your environment. There are a few guidelines that we need contributors to
follow so that we can have a chance of keeping on top of things.

## Hercules Core vs Modules

Due to the nature of the project and the wide range of different applications
it has, we provide a plugin interface to keep the core clean of unnecessary
features.

Generally, bugfixes and improvements to existing code, as well as the
implementation of official Ragnarok Online features and content, should be part
of the Hercules core while custom functionalities should be moved to plugins
to avoid burdening the core with code that is potentially useful to only a small subset
of users.

If you are unsure of whether your contribution should be implemented as a
module or part of Hercules Core, you may visit [#Hercules on Rizon
IRC](http://herc.ws/board/topic/91-hercules-irc/), create an issue on GitHub,
or drop us an email at dev@herc.ws

## Getting Started

* Make sure you have a [GitHub account](https://github.com/signup/free)
* Open an issue in GitHub, if one does not already exist.
  * Clearly describe the issue including steps to reproduce when it is a bug.
  * Describe your configuration, following the provided template.
* Fork the repository on GitHub

## Submitting an Issue on GitHub

When you open an issue, you should include as much description as possible of 
the issue you are observing or feature you're suggesting.

If you're reporting an issue, you should describe your setup, and provide the
output of `./map-server --version`.

If you report a crash, make sure that you include a backtrace of the crash
generated with either gdb or Visual Studio (depending on your build
environment). For the backtrace to be useful, you need to compile Hercules in
debug mode.

## Making Changes

* Create a topic branch from where you want to base your work.
  * This is usually the master branch.
  * To quickly create a topic branch based on master; `git checkout -b
    my_contribution master`. Please avoid working directly on the
    `master` branch.
* Make commits of logical units. Each commit you submit must be atomic and
  complete. **Each commit must do one thing, and do it well.** Make separate commits 
  for separate fixes, even if this causes commits that only affect one line of code.
* Check for unnecessary whitespace with `git diff --check` before committing.
* Make sure you follow our [coding style
  guidelines](https://github.com/HerculesWS/Hercules/wiki/Coding-Style).
* Make sure your commit messages are complete, describe the changes you made,
  and in proper English language. Make sure you mention the ID of the issue
  you fix.
* Make sure your changes don't accidentally break anything when, for example,
  Hercules is compiled with different settings.

### Making Trivial Changes

For changes of a trivial nature to comments and documentation, it is not always
necessary to create a new issue in GitHub.

## Submitting Changes

* Push your changes to a topic branch in your fork of the repository.
* Submit a pull request to the repository in the HerculesWS organization.
* The dev team looks at Pull Requests on a weekly basis, compatibly with the
  amount of patches in review queue and current workload.
* After feedback has been given, we expect responses within two weeks. After two
  weeks we may close the pull request if it isn't showing any activity.

## Machine Assisted Edits

If you use tools to help you edit or generate the contents you're contributing,
we require you to adopt special measures to help us review your contributions.
Examples of such tools include but are not limited to:

* global search/replace functionality in IDEs
* auto-fix functionality provided by linters
* "AI" tools (LLMs, assistants, etc.) used to generate code or documentation

If you used any such tools in your process, please explain the nature of the
tools in your Pull Request description. Put the reviewers in the conditions to
reproduce your automation on their end (such as the search and replace strings
or regular expressions you applied, or if possible any LLM prompts you used).

Please keep in mind that, regardless of the tools used, you, as the submitter of
the Pull Request, still have the responsibility to ensure the correctness of
your changes (and to fully understand them, at least to the best of your
knowledge), and, in case of large Pull Requests, that the value added to the
project by your contribution counterbalances the effort that the team members
will have to go through to review and merge your changes. When in doubt, you may
create the Pull Request in the draft state, and seek the team's assistance.

If the reviewers determine that the complexity or size of your Pull Request
outweighs the benefits it provides to the project, they may decide to close it,
referring to this document.

## "AI" Tools

As mentioned in the "Machine Assisted Edits" section, the use of AI tools to
assist you while writing a Pull Request is permitted, but as the submitter
you're required to fully understand your proposed changes (or at least to the
best of your knowledge, we don't require everyone to fully understand every
obscure bit of the emulator).

Additionally, if you used AI tools (LLMs, IDE assistants, etc.) as part of the
process that led to your Pull Request, you must disclose it within the PR's
description. A brief paragraph mentioning the tool or model you used is
required, and listing your prompts is strongly recommended, as it will greatly
help the reviewers to understand and validate the changes.

Please note that listing the tool as co-author or co-signer in your commit
messages is not allowed. You remain the sole author of your commits, regardless
of the tools you used to generate them.

Large Pull Requests entirely generated by AI tools are not allowed.

Commit messages and Pull Request descriptions generated by AI tools are not
allowed. You must understand the changes, and describe them in the commit
message in your own words. Even if you're not a native English speaker, having
perfect English grammar in your commit messages is to us less valuable than
having the author's own thoughts and reasons described in their own words.

When submitting AI-generated or AI-assisted content, please don't let the
reviewers have the first look at the code. The current state of the art of these
tools is not perfect, and there may be glaring issues in the generated code.
Verify the changes yourself before you submit a Pull Request (including
reviewing the code, testing, trying to fully understand the implications of each
change). Reviewers may ask questions about your AI-assisted code, and if you
should always be able to explain why each change was made and justify it, or the
PR may be rejected even if you believe it is ultimately correct.

The use of AI tools in the comments section of the Pull Requests is strictly
forbidden. Reviewers have access to the same AI tools that you have access to,
and if they ask you anything about your PR, they do so because they need to hear
your response rather than the AI tool's that they could have otherwise just
queried on their own. If you are not able to honestly answer their questions,
please admit so, rather than wiring up an LLM's response, or your PR will be
closed.

## Other ways to help

* You can help us diagnose and fix existing bugs by asking and providing answers for the following:

  * Is the bug reproducible as explained?
  * Is it reproducible in other environments?
  * Are the steps to reproduce the bug clear? If not, can you describe how you might reproduce it?
  * Is this bug something you have run into? Would you appreciate it being looked into faster?

* You can close fixed bugs by testing old bugs to see if they are still happening.
