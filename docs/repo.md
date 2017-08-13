## Submit Your Module to Magisk Modules Repo
If you want to share your module with others, you can submit your modules to [Magisk Modules Repo](https://github.com/Magisk-Modules-Repo). In order to submit a module, you will need to know how to use some basic `git`.

1. Create a module as stated [here](module.md), and test if it works properly
1. Fork [this repo](https://github.com/topjohnwu/magisk-module-template) to your account
1. Commit and push your changes to your forked repo
1. Open an issue in [topjohnwu/Magisk_Repo_Central](https://github.com/topjohnwu/Magisk_Repo_Central/issues/new) with your repo link
1. I will review your module, and once accepted, your repo should be cloned into [Magisk-Modules-Repo](https://github.com/Magisk-Modules-Repo), and you should receive an email to become the collaborator so you can edit the repo in the future.

#### Once your module is live on the Modules Repo, the description of your repo should be the ID of your module. Please do NOT change the description, repeat, do NOT change the description.

## Notes

- Magisk Manager monitors all repo's `master` branch. Any changes to the branch `master` will be reflected to all users immediately. If you are working on an update for a module, please work on another branch, make sure it works, then finally merge the changes back to `master`.
- Once you finished upgrading your repo, bump up at least the `versionCode` in `module.prop`, so Magisk Manager will know you module is updated!
- The description of your repo should be the same as your module ID. If you changed your description, Magisk Manager will fail to identify your repo, and cannot relate installed module to the online repo together.