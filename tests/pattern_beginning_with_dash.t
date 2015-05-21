Setup:

  $ . $TESTDIR/setup.sh
  $ echo 'function get_thread($api) {'       > ./psuedocode.txt
  $ echo '    $x = $api->thread->get();'    >> ./psuedocode.txt
  $ echo '    return $x->thread_base->url;' >> ./psuedocode.txt
  $ echo '}'                                >> ./psuedocode.txt

Search for lines matching "->thread->" in psuedocode.txt:

  $ ag -e '->thread->'
  psuedocode.txt:2:    return $api->thread->get();

Search for lines matching "->thread" in psuedocode.txt:

  $ ag -e '->thread'
  psuedocode.txt:2:    $x = $api->thread->get();
  psuedocode.txt:3:    return $x->thread_base->url;

Search for lines matching "->thread.*->" in psuedocode.txt:

  $ ag -e '->thread'
  psuedocode.txt:2:    $x = $api->thread->get();
  psuedocode.txt:3:    return $x->thread_base->url;
