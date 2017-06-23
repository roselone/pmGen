int _flag[2];
int _turn;
int test;

proctype _P0(int t; chan __return) {
int retval;
int t_addr;
int tmp;
bit cmp;
int tmp1;
bit cmp2;
bit v0;
int tmp3;
int inc;
int tmp4;
int call;
int tmp5;
int dec;
int v1;
int currentLabel;
chan _syn = [0] of { int };

Label0:
  currentLabel = 0;
  t_addr = t;
   _flag[0] = 1;
  _turn = 1;
  goto Label1;

Label1:
  currentLabel = 1;
  tmp =  _flag[1];
  cmp = (tmp == 1);
  if
    ::(cmp!= 0) -> goto Label2
    ::(cmp==0) -> goto Label3
  fi;

Label2:
  currentLabel = 2;
  tmp1 = _turn;
  cmp2 = (tmp1 == 1);
  goto Label3;

Label3:
  if
    ::(currentLabel == 1)->v0 = false
    ::(currentLabel == 2)->v0 = cmp2
  fi;
  currentLabel = 3;
  if
    ::(v0!= 0) -> goto Label4
    ::(v0==0) -> goto Label5
  fi;

Label4:
  currentLabel = 4;
  goto Label1;

Label5:
  currentLabel = 5;
  tmp3 = test;
  inc = tmp3 + 1;
  test = inc;
  tmp4 = test;
   printf( "%d\0A\00" , tmp4);
  tmp5 = test;
  dec = tmp5 + -1;
  test = dec;
   _flag[0] = 0;
  v1 = retval;
  __return!v1;
  goto LabelSkip;
LabelSkip:skip
}

proctype _P1(int t; chan __return) {
int retval;
int t_addr;
int tmp;
bit cmp;
int tmp1;
bit cmp2;
bit v0;
int tmp3;
int inc;
int tmp4;
int call;
int tmp5;
int dec;
int v1;
int currentLabel;
chan _syn = [0] of { int };

Label0:
  currentLabel = 0;
  t_addr = t;
   _flag[1] = 1;
  _turn = 0;
  goto Label1;

Label1:
  currentLabel = 1;
  tmp =  _flag[0];
  cmp = (tmp == 1);
  if
    ::(cmp!= 0) -> goto Label2
    ::(cmp==0) -> goto Label3
  fi;

Label2:
  currentLabel = 2;
  tmp1 = _turn;
  cmp2 = (tmp1 == 0);
  goto Label3;

Label3:
  if
    ::(currentLabel == 1)->v0 = false
    ::(currentLabel == 2)->v0 = cmp2
  fi;
  currentLabel = 3;
  if
    ::(v0!= 0) -> goto Label4
    ::(v0==0) -> goto Label5
  fi;

Label4:
  currentLabel = 4;
  goto Label1;

Label5:
  currentLabel = 5;
  tmp3 = test;
  inc = tmp3 + 1;
  test = inc;
  tmp4 = test;
   printf( "%d\0A\00" , tmp4);
  tmp5 = test;
  dec = tmp5 + -1;
  test = dec;
   _flag[1] = 0;
  v1 = retval;
  __return!v1;
  goto LabelSkip;
LabelSkip:skip
}

proctype _main(chan __return) {
int retval;
int p0;
int p1;
int t;
int tmp;
int conv;
int v0;
int call;
int tmp1;
int conv2;
int v1;
int call3;
int currentLabel;
chan _syn = [0] of { int };
chan _return0 = [0] of { int };
chan _return1 = [0] of { int };

Label0:
  currentLabel = 0;
  retval = 0;
  t = 0;
  tmp = t;
  conv =  tmp;
  v0 =  conv;
  run _P0(v0, _return0);
  tmp1 = t;
  conv2 =  tmp1;
  v1 =  conv2;
  run _P1(v1, _return1);
  __return!0;
  goto LabelSkip;
LabelSkip:skip
}
init {
chan _syn = [0] of { int };
  _turn = 0;
  test = 0;
  run _main(_syn);
}

ltl p1 { [] ( test != 2 ) }
