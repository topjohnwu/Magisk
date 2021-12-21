# How to release

* Make sure you're on master and synced to HEAD
* Ensure the project builds and tests run (sanity check only, obviously)
    * `parallel -j0 exec ::: test/*_test` can help ensure everything at least
      passes
* Prepare release notes
    * `git log $(git describe --abbrev=0 --tags)..HEAD` gives you the list of
      commits between the last annotated tag and HEAD
    * Pick the most interesting.
* Create a release through github's interface
    * Note this will create a lightweight tag.
    * Update this to an annotated tag:
      * `git pull --tags`
      * `git tag -a -f <tag> <tag>`
      * `git push --force origin`
