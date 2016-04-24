# Contributing to Hercules

Hello! Third party patches are essential to keep Hercules great. We want to
keep it as easy as possible to contribute changes that get things working in
your environment. There are a few guidelines that we need contributors to
follow so that we can have a chance of keeping on top of things.

## Hercules Core vs Modules

Due to the nature of the project, and the wide range of different applications
it has, we provide a plugin interface, to keep the core clean of unnecessary
features.

Generally, bugfixes and improvements to existing code, as well as the
implementation of official Ragnarok Online features and content, should be part
of the Hercules core, while custom functionalities should be moved to plugins,
to avoid burdening the core with code potentially useful only to a small subset
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

When you open an issue, in order for it to be helpful, you should include as
much description as possible of the issue you are observing or feature you're
suggesting.

If you're reporting an issue, you should describe your setup, and provide the
output of `./map-server --version`.

If you report a crash, make sure that you include a backtrace of the crash,
generated with either gdb or Visual Studio (depending on your build
environment). For the backtrace to be useful, you need to compile Hercules in
debug mode.

## Making Changes

* Create a topic branch from where you want to base your work.
  * This is usually the master branch.
  * To quickly create a topic branch based on master; `git checkout -b
    my_contribution master`. Please avoid working directly on the
    `master` branch.
* Make commits of logical units. Each commit you submit, must be atomic and
  complete. **Each commit must do one thing, and do it well.** For separate
  fixes, make separate commits. Even if this causes commits that only affect
  one line of code.
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
* After feedback has been given we expect responses within two weeks. After two
  weeks we may close the pull request if it isn't showing any activity.

## Other ways to help

* You can help us diagnose and fix existing bugs by asking and providing answers for the following:

  * Is the bug reproducible as explained?
  * Is it reproducible in other environments?
  * Are the steps to reproduce the bug clear? If not, can you describe how you might reproduce it?
  * Is this bug something you have run into? Would you appreciate it being looked into faster?

* You can close fixed bugs by testing old bugs to see if they are still happening.
