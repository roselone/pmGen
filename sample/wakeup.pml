int _isRunning;
int _lk;
int _r_lock;
int _r_want;
int _sleep_q;

proctype _client(int n; chan __return) {
int retval;
int n_addr;
int tmp;
bit tobool;
int conv;
bit cmp;
int tmp3;
bit tobool4;
int conv5;
bit cmp6;
int tmp10;
bit tobool11;
bit lnot;
int v0;
int currentLabel;
chan _syn = [0] of { int };

Label0:
  currentLabel = 0;
  n_addr = n;
  goto Label1;

Label1:
  currentLabel = 1;
  goto Label2;

Label2:
atomic{
  currentLabel = 2;
  tmp = _lk;
  tobool =  tmp;
  conv =  tobool;
  cmp = (conv != 0);
  if
    ::(cmp!= 0) -> goto Label2
    ::(cmp==0) -> _lk=1; goto Label5
  fi;
}

Label3:
  currentLabel = 3;
  goto Label2;

Label4:
  currentLabel = 4;
  _lk = 1;
  goto Label5;

Label5:
  currentLabel = 5;
  tmp3 = _r_lock;
  tobool4 =  tmp3;
  conv5 =  tobool4;
  cmp6 = (conv5 == 1);
  if
    ::(cmp6!= 0) -> goto Label6
    ::(cmp6==0) -> goto Label10
  fi;

Label6:
  currentLabel = 6;
  _r_want = 1;
  _isRunning = 0;
  _lk = 0;
  goto Label7;

Label7:
  currentLabel = 7;
  tmp10 = _isRunning;
  tobool11 =  tmp10;
  lnot = tobool11 ^ true;
  if
    ::(lnot!= 0) -> goto Label8
    ::(lnot==0) -> goto Label9
  fi;

Label8:
  currentLabel = 8;
  goto Label7;

Label9:
  currentLabel = 9;
  goto Label5;

Label10:
  currentLabel = 10;
  goto Label11;

progress:
Label11:
  currentLabel = 11;
  _r_lock = 1;
  _lk = 0;
  goto Label1;

Label12:
  currentLabel = 12;
  v0 = retval;
  __return!v0;
  goto LabelSkip;
LabelSkip:skip
}

proctype _server(int n; chan __return) {
int retval;
int n_addr;
int tmp;
bit tobool;
int conv;
bit cmp;
int tmp2;
bit tobool3;
int tmp5;
bit tobool6;
int conv7;
bit cmp8;
int tmp13;
bit tobool14;
int conv15;
bit cmp16;
int tmp20;
bit tobool21;
int v0;
int currentLabel;
chan _syn = [0] of { int };

Label0:
  currentLabel = 0;
  n_addr = n;
  goto Label1;

Label1:
  currentLabel = 1;
  _r_lock = 0;
  goto Label2;

Label2:
  currentLabel = 2;
  tmp = _lk;
  tobool =  tmp;
  conv =  tobool;
  cmp = (conv != 0);
  if
    ::(cmp!= 0) -> goto Label3
    ::(cmp==0) -> goto Label4
  fi;

Label3:
  currentLabel = 3;
  goto Label2;

Label4:
  currentLabel = 4;
  tmp2 = _r_want;
  tobool3 =  tmp2;
  if
    ::(tobool3!= 0) -> goto Label5
    ::(tobool3==0) -> goto Label14
  fi;

Label5:
  currentLabel = 5;
  goto Label6;

Label6:
  currentLabel = 6;
  tmp5 = _sleep_q;
  tobool6 =  tmp5;
  conv7 =  tobool6;
  cmp8 = (conv7 != 0);
  if
    ::(cmp8!= 0) -> goto Label7
    ::(cmp8==0) -> goto Label8
  fi;

Label7:
  currentLabel = 7;
  goto Label6;

Label8:
  currentLabel = 8;
  _sleep_q = 1;
  _r_want = 0;
  goto Label9;

Label9:
  currentLabel = 9;
  tmp13 = _lk;
  tobool14 =  tmp13;
  conv15 =  tobool14;
  cmp16 = (conv15 != 0);
  if
    ::(cmp16!= 0) -> goto Label10
    ::(cmp16==0) -> goto Label11
  fi;

Label10:
  currentLabel = 10;
  goto Label9;

Label11:
  currentLabel = 11;
  tmp20 = _isRunning;
  tobool21 =  tmp20;
  if
    ::(tobool21!= 0) -> goto Label13
    ::(tobool21==0) -> goto Label12
  fi;

Label12:
  currentLabel = 12;
  _isRunning = 1;
  goto Label13;

Label13:
  currentLabel = 13;
  _sleep_q = 0;
  goto Label14;

Label14:
  currentLabel = 14;
  goto Label1;

Label15:
  currentLabel = 15;
  v0 = retval;
  __return!v0;
  goto LabelSkip;
LabelSkip:skip
}

proctype _main(chan __return) {
int retval;
int p0;
int p1;
int t;
int v0;
int call;
int v1;
int call1;
int currentLabel;
chan _syn = [0] of { int };
chan _return0 = [0] of { int };
chan _return1 = [0] of { int };

Label0:
  currentLabel = 0;
  retval = 0;
  t = 0;
  v0 =  t;
  run _client(v0, _return0);
  v1 =  t;
  run _server(v1, _return1);
  __return!0;
  goto LabelSkip;
LabelSkip:skip
}
init {
chan _syn = [0] of { int };
  _isRunning = 1;
  _lk = 0;
  _r_lock = 0;
  _r_want = 0;
  _sleep_q = 0;
  run _main(_syn);
}

