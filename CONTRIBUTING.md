## Contributing

I like when people send pull requests. It validates my existence. If you want to help out, check the [issue list](https://github.com/ggreer/the_silver_searcher/issues?sort=updated&state=open) or search the codebase for `TODO`. Don't worry if you lack experience writing C. If I think a pull request isn't ready to be merged, I'll give feedback in comments. Once everything looks good, I'll comment on your pull request with a cool animated gif and hit the merge button.

### Running the test suite

If you contribute, you might want to run the test suite before and after writing
some code, just to make sure you did not break anything. Adding tests along with
your code is nice to have, because it makes regressions less likely to happen.
Also, if you think you have found a bug, contributing a failing test case is a
good way of making your point and adding value at the same time.

The test suite uses [Cram](https://bitheap.org/cram/). You'll need to build ag
first, and then you can run the suite from the root of the repository :

    make test

You can run a single test manually with cram, like: `cram -v tests/option_e.t`
