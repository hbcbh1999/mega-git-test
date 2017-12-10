# mega-git-test

A quick experiment how git handles a Google-sized code base:

* millions of files
* billions of code lines


## Story

Today's Hacker News [re-post](https://news.ycombinator.com/item?id=15889148) of [_"Why Google stores billions of lines of code in a single repository"_](https://dl.acm.org/citation.cfm?id=2854146) promted me to do this.

In there, it is discussed how

> a monorepo this size would simply not scale on git, at least not without huge amounts of hacks

How much is it?

> ... two billion lines of code in nine million unique source files ... ([source](https://cacm.acm.org/magazines/2016/7/204032-why-google-stores-billions-of-lines-of-code-in-a-single-repository/fulltext))

Note this includes only source code, ignoring Google's habit of storing built binaries in the repository.

So I wanted to find out how git would deal with that amount of data.
Obviously I'm not suggesting here that Google could easily run their repo on top of git; there are constraints beyond the speed of basic operations like `git status` and `git diff` that are harder to fulfill, such as how many pushes per second must be handle and so on.
Further, I'm not currently testing any history operations here yet (experience reports welcome).
In this experiement, we just want to get a rough idea of how some basic git operations perform on a large code repository.


## Running git with 1 M files and 200 M lines, on my laptop

All of the below happened on my Thinkpad X220 from 2011 with an SSD inside, running Ubuntu 16.04.

Right now I don't have enough disk space free on the SSD, so I'm testing at approximately 10% of the target repository size:

|              | files      | lines of code |
|--------------|-----------:|--------------:|
| target scale | 10 million | 2   billion   | 
| tested scale |  1 million | 0.2 billion   | 

The experiment so far suggests that git operations I do here scale linearly, so I'd expect all timings to be 10x larger for the full size (but may confirm that once I've made some more space on the SSD).

We assume each line has 100 characters on it.

`create.c` is a quick-and-dirty C loop that creates above mentioned files.
To run this experiement at target scale instead of at 10%, simply remove the `/ 10` in the first few lines.

### Execution

```
$ mkdir repo    # because create.c is so pityable that it doesn't even mkdir()

$ make
gcc -O2 -g -std=c99 create.c -o create
time ./create
Creating 900000 files
  222 lines each
  199800000 total lines
  100 chars per line
  19.980000 total GB
44.62user 31.00system 1:46.92elapsed 70%CPU (0avgtext+0avgdata 1464maxresident)k
```

OK, not bad, creating the 0.2 billion LoC took just under 2 minutes.

```
$ du -sh .
21.4G .
```

At full target scale, this would be around 250 GB.
That's a lot of code, but would still comfortably fit on an empty laptop SSD.

```
$ git add . > /dev/null
$ time git commit -m "initial commit" > /dev/null

real  5m11.401s
user  1m10.120s
sys 0m34.480s
```

OK, doing the initial import commit took 5 minutes, so I'd expect 50 minutes for the full scale test.

```
$ du -sh repo/.git
3.6G	repo/.git
```

Even though no two lines in the repository are equal (they are generated like `file 0000006468 line 0001 - Lorem ipsum dolor sit amet ...`) git still managed to compress its store by factor 6 compared to the size of our tree.

```
$ time git status
On branch master
nothing to commit, working directory clean

real  0m1.769s
user  0m1.188s
sys 0m0.884s
```

That's nice, `git status` takes < 2 seconds to run.
At full scale ~15 seconds would start to get slighly annoying, but probably still workable.

Let's change a file and see how quickly `git status` notices the change:

```
$ ls -1U | head -n3
file-0000516230
file-0000238910
file-0000405680

$ echo "another line" >> file-0000516230

$ time git status
On branch master
Changes not staged for commit:

  modified:   file-0000516230

real  0m1.766s
```

Nice, finding the change is about as fast as a no-op. Does `git diff` do it that quickly as well?

```
$ time git diff
diff --git a/file-0000516230 b/file-0000516230
index 2d3f8c5..7fd21da 100644
--- a/file-0000516230
+++ b/file-0000516230
@@ -219,4 +219,4 @@ file 0000516230 line 0218 - Lorem ipsum dolor sit amet, consectetur adipiscing e
 file 0000516230 line 0219 - Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
 file 0000516230 line 0220 - Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
 file 0000516230 line 0221 - Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
-file 0000516230 line 0222 - Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
\ No newline at end of file
+file 0000516230 line 0222 - Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmodanother line

real  0m0.665s
user  0m0.376s
sys 0m0.600s
```

Yes. Conveniently `git diff` is even faster than `git status`.
Even at "full Google scale", 5 seconds `git diff` is still something I could live with.

Now let us commit the change:

```
$ time git commit -a -m "changing a line"
[master 2e574b8] changing a line
 1 file changed, 1 insertion(+), 1 deletion(-)

real	0m4.647s
user	0m3.032s
sys	0m1.876s
```

OK, 5 seconds seems acceptable.
If the full scale made this take 50 seconds, it would be annoying, but also still workable.

Let's look at some history:

```
$ time git log
commit 2e574b8cafb4c8d74fee1eded476ba3de99dc83f
Author: Niklas Hambüchen <mail@nh2.me>
Date:   Sun Dec 10 18:36:19 2017 +0100

    changing a line

commit 23c2ab8a30483d18da893124b1d6cbb7500fd201
Author: Niklas Hambüchen <mail@nh2.me>
Date:   Sun Dec 10 17:35:09 2017 +0100

    initial commit

real	0m0.009s
user	0m0.000s
sys	0m0.004s
```

Ah nice, meta-information is fast as we would hope for.

This concludes the testing I managed to get done within 2 hours of my weekend.
If anybody wants to test this on a slightly larger SSD, please file an issue and report your findings!


## Summary

* For 0.2 BLoC, the basic git operations tested still finish comfortably within a few seconds.
* For "full Google scale" (10x my test size), I'd expect them to finish in under 1 minute; annoying, but not forbiddingly slow.
* A laptop from 2011 happily executes such tests.
* It would be interesting to see how the numbers change when things below userspace are swapped out; for example, it would be nice to add a `statv()` syscall to stat many files in one go, or test on a different storage technology, such as Intel Optane memory like [this one](https://www.phoronix.com/scan.php?page=article&item=intel-optane-900p&num=1). Please file your findings as issues!
