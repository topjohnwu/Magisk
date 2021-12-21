# libbase

## Who is this library for?

This library is a collection of convenience functions to make common tasks
easier and less error-prone.

In this context, "error-prone" covers both "hard to do correctly" and
"hard to do with good performance", but as a general purpose library,
libbase's primary focus is on making it easier to do things easily and
correctly when a compromise has to be made between "simplest API" on the
one hand and "fastest implementation" on the other. Though obviously
the ideal is to have both.

## Should my routine be added?

The intention is to cover the 80% use cases, not be all things to all users.

If you have a routine that's really useful in your project,
congratulations. But that doesn't mean it should be here rather than
just in your project.

The question for libbase is "should everyone be doing this?"/"does this
make everyone's code cleaner/safer?". Historically we've considered the
bar for inclusion to be "are there at least three *unrelated* projects
that would be cleaned up by doing so".

If your routine is actually something from a future C++ standard (that
isn't yet in libc++), or it's widely used in another library, that helps
show that there's precedent. Being able to say "so-and-so has used this
API for n years" is a good way to reduce concerns about API choices.

## Any other restrictions?

Unlike most Android code, code in libbase has to build for Mac and
Windows too.

Code here is also expected to have good test coverage.

By its nature, it's difficult to change libbase API. It's often best
to start using your routine just in your project, and let it "graduate"
after you're certain that the API is solid.
