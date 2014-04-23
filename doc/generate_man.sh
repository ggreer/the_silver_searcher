#!/bin/sh

# ronn is used to turn the markdown into a manpage.
# Get ronn at https://github.com/rtomayko/ronn

awk '
BEGIN{
  in_options_block = 0;
  first_item_in_list_of_options = 1;
}

{
  if ($0 == "## OPTIONS") {
    in_options_block = 1;
  }

  if (in_options_block == 1) { # in options block
    first_4_chars = substr($0, 0, 4);

    if (first_4_chars == "  * ") { # this line contains the option name

      # print only 1 new line for cases like the following
      # * `--[no]group`
      # * `-g PATTERN`:
      if (first_item_in_list_of_options == 1) {
        print "";
        first_item_in_list_of_options = 0;
      }

      # end the line with 2 spaces, so a literal <br> is inserted!
      # more info at http://daringfireball.net/projects/markdown/syntax.php#p
      printf("%s  \n", substr($0, 5));

    } else if (first_4_chars == "    ") { # we are in a description line
      printf("&nbsp;&nbsp;&nbsp;&nbsp;  %s\n", substr($0, 5));
      first_item_in_list_of_options = 1;
    } else if (first_4_chars == "## F") { # reached the end of #OPTIONS part
      in_options_block = 0;
      print $0;
    } else {
      print $0;
    }
  } else { # outside options block
    print $0;
  }
}' <ag.1.md >ag.1.md.tmp

ronn -r ag.1.md.tmp

rm -f ag.1.md.tmp
