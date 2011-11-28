Just kidding. This thing is nowhere close to ack.

1. Make it compile.
2. Make it run.
3. Make it behave correctly.
4. Make it fast.

It's somewhere between 2 and 3 right now.

notes:  
ack annoyances:  
  only uses one core  
  can't prioritize certain dirs/files  
  can't say "just ignore stuff that's in .gitignore/.svnignore/.hgignore/whatever"  
  can't easily introspect what files are being ignored/searched  
  poor/no support for searching/ignoring files with no dots in their name or dots at the beginning or multiple dots, etc  
  ack is a giant hack. it's full of crazy stuff that perl programmers do. tons of special-cases and other dumb stuff  
  ack hates anything that isn't utf-8  
  by default, it hardly searches anything. you have to edit your ~/.ackrc to get useful behavior  
    grep is on the opposite side of the spectrum  
  no easy way to show skipped files in a directory hierarchy  
  if there are no matches, ack doesn't automatically try some less accurate searches  
    concrete example: if no matches, enable ignore case and redo the search  
  basic options:  
    follow/don't follow symlinks  
    recurse dirs  

ack awesomeness (aka stuff I want to copy):  
  can easily skip files (unlike grep)  
  faster than grep  
  integrates with textmate (ackmate)  
  
code search: common actions  
  find vars or functions with same/similar names in the project  

pie-in-the-sky stuff  
  integrate with filetype-specific libraries (libxml2 etc) for extra functionality  
    concrete example: searching some xml files  
      instead of printing the context lines (prev/next 5 lines), print everything in the local dom scope  
    track code dependencies (this is almost certainly a different product/program)  
