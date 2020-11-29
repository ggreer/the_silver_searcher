Setup:

  $ . $TESTDIR/setup.sh
  $ printf "\nvoid a(){}\nvoid b(){\n  a;\n  b;\n  c;\n  d;\n}\nvoid c(){\n  e;\n  f;\n  g;\n  h;\n  i;\n  g;\n  k;\n}\nvoid d(){\n}\nvoid x(){}\nvoid y(){}\n\nvoid z(){}\n" > foobar.c

Ensure --multiline-break is correct:
  $ ag --multiline-break  '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  --
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  --
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  --
  foobar.c:18:void d(){
  foobar.c:19:}
  --
  foobar.c:20:void x(){}
  --
  foobar.c:21:void y(){}
  --
  foobar.c:23:void z(){}


Ensure --multilinebreak is correct:
  $ ag --multilinebreak  '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  --
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  --
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  --
  foobar.c:18:void d(){
  foobar.c:19:}
  --
  foobar.c:20:void x(){}
  --
  foobar.c:21:void y(){}
  --
  foobar.c:23:void z(){}

Ensure --multilinebreak --multiline is correct:
  $ ag --multilinebreak --multiline '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  --
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  --
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  --
  foobar.c:18:void d(){
  foobar.c:19:}
  --
  foobar.c:20:void x(){}
  --
  foobar.c:21:void y(){}
  --
  foobar.c:23:void z(){}

Ensure --multiline-break --multiline is correct:
  $ ag --multiline-break --multiline '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  --
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  --
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  --
  foobar.c:18:void d(){
  foobar.c:19:}
  --
  foobar.c:20:void x(){}
  --
  foobar.c:21:void y(){}
  --
  foobar.c:23:void z(){}


Ensure --nomultilinebreak --multiline is correct:
  $ ag --nomultilinebreak --multiline '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  foobar.c:18:void d(){
  foobar.c:19:}
  foobar.c:20:void x(){}
  foobar.c:21:void y(){}
  foobar.c:23:void z(){}


Ensure --no-multiline-break --multiline is correct:
  $ ag --no-multiline-break --multiline '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  foobar.c:18:void d(){
  foobar.c:19:}
  foobar.c:20:void x(){}
  foobar.c:21:void y(){}
  foobar.c:23:void z(){}

Ensure --nomultilinebreak is correct:
  $ ag --nomultilinebreak  '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  foobar.c:18:void d(){
  foobar.c:19:}
  foobar.c:20:void x(){}
  foobar.c:21:void y(){}
  foobar.c:23:void z(){}

Ensure --no-multiline-break --multiline is correct:
  $ ag --no-multiline-break --multiline '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  foobar.c:18:void d(){
  foobar.c:19:}
  foobar.c:20:void x(){}
  foobar.c:21:void y(){}
  foobar.c:23:void z(){}

Ensure --no-multiline-break --nomultiline is correct:
  $ ag --no-multiline-break --nomultiline '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  foobar.c:20:void x(){}
  foobar.c:21:void y(){}
  foobar.c:23:void z(){}

Ensure --noheading  --multilinebreak is correct:
  $ ag --noheading  --multilinebreak  '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  --
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  --
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  --
  foobar.c:18:void d(){
  foobar.c:19:}
  --
  foobar.c:20:void x(){}
  --
  foobar.c:21:void y(){}
  --
  foobar.c:23:void z(){}

Ensure --noheading  --multiline-break is correct:
  $ ag --noheading  --multiline-break  '(.*?((?::{2})?(?:\b[A-Za-z_]\w*(?::{2}))*[~]?[A-Za-z_]\w*)(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*}))'  
  foobar.c:2:void a(){}
  --
  foobar.c:3:void b(){
  foobar.c:4:  a;
  foobar.c:5:  b;
  foobar.c:6:  c;
  foobar.c:7:  d;
  foobar.c:8:}
  --
  foobar.c:9:void c(){
  foobar.c:10:  e;
  foobar.c:11:  f;
  foobar.c:12:  g;
  foobar.c:13:  h;
  foobar.c:14:  i;
  foobar.c:15:  g;
  foobar.c:16:  k;
  foobar.c:17:}
  --
  foobar.c:18:void d(){
  foobar.c:19:}
  --
  foobar.c:20:void x(){}
  --
  foobar.c:21:void y(){}
  --
  foobar.c:23:void z(){}
