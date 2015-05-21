Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'function get_thread($api) {'       > ./psuedocode.txt
  $ echo '    $x = $api->thread->get();'    >> ./psuedocode.txt
  $ echo '    return $x->thread_base->url;' >> ./psuedocode.txt
  $ echo '}'                                >> ./psuedocode.txt

Search for lines matching "->thread->" in psuedocode.txt:

  $ ag -e '->thread->'
  psuedocode.txt:2:    $x = $api->thread->get();

Search for lines matching "->thread" in psuedocode.txt:

  $ ag -e '->thread'
  psuedocode.txt:2:    $x = $api->thread->get();
  psuedocode.txt:3:    return $x->thread_base->url;

Search for lines matching "->thread.*->" in psuedocode.txt:

  $ ag -e '->thread'
  psuedocode.txt:2:    $x = $api->thread->get();
  psuedocode.txt:3:    return $x->thread_base->url;

Ensure usage mentions the option:

  $ ag -h | grep -e '-e --regexp PATTERN' >/dev/null

Same as prior search for "->thread->", but use "--regexp":

  $ ag --regexp '->thread->'
  psuedocode.txt:2:    $x = $api->thread->get();

Same as prior search for "->thread", but use "--regexp":

  $ ag --regexp '->thread'
  psuedocode.txt:2:    $x = $api->thread->get();
  psuedocode.txt:3:    return $x->thread_base->url;

Same as prior search for "->thread.*->", but use "--regexp":

  $ ag --regexp '->thread'
  psuedocode.txt:2:    $x = $api->thread->get();
  psuedocode.txt:3:    return $x->thread_base->url;
