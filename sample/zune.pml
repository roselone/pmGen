proctype _year(int day; chan __return) {
int retval;
int day_addr;
int days;
int year;
int tmp;
int v0;
int tmp1;
int tmp3;
bit cmp;
int tmp4;
int rem;
bit cmp5;
int tmp6;
int rem7;
bit cmp8;
int tmp9;
int rem10;
bit cmp11;
int tmp12;
bit cmp13;
int tmp15;
int sub;
int tmp16;
int add;
int tmp17;
int sub18;
int tmp19;
int add20;
int tmp22;
int tmp23;
int call;
int v1;
int currentLabel;
chan _syn = [0] of { int };

Label0:
  currentLabel = 0;
  day_addr = day;
  tmp = day_addr;
  v0 =  tmp;
  tmp1 = v0;
  days = tmp1;
  year = 1980;
  goto Label1;

Label1:
  currentLabel = 1;
  tmp3 = days;
  cmp = (tmp3 > 365);
  if
    ::(cmp!= 0) -> goto Label2
    ::(cmp==0) -> goto Label10
  fi;

Label2:
  currentLabel = 2;
  tmp4 = year;
  rem = tmp4 % 4;
  cmp5 = (rem == 0);
  if
    ::(cmp5!= 0) -> goto Label3
    ::(cmp5==0) -> goto Label4
  fi;

Label3:
  currentLabel = 3;
  tmp6 = year;
  rem7 = tmp6 % 100;
  cmp8 = (rem7 != 0);
  if
    ::(cmp8!= 0) -> goto Label5
    ::(cmp8==0) -> goto Label4
  fi;

Label4:
  currentLabel = 4;
  tmp9 = year;
  rem10 = tmp9 % 400;
  cmp11 = (rem10 == 0);
  if
    ::(cmp11!= 0) -> goto Label5
    ::(cmp11==0) -> goto Label8
  fi;

Label5:
  currentLabel = 5;
  tmp12 = days;
  cmp13 = (tmp12 > 366);
  if
    ::(cmp13!= 0) -> goto Label6
    ::(cmp13==0) -> goto Label7
  fi;

Label6:
  currentLabel = 6;
  tmp15 = days;
  sub = tmp15 - 366;
  days = sub;
  tmp16 = year;
  add = tmp16 + 1;
  year = add;
  goto Label7;

Label7:
  currentLabel = 7;
  goto Label9;

Label8:
  currentLabel = 8;
  tmp17 = days;
  sub18 = tmp17 - 365;
  days = sub18;
  tmp19 = year;
  add20 = tmp19 + 1;
  year = add20;
  goto Label9;

Label9:
  currentLabel = 9;
  goto Label1;

Label10:
  currentLabel = 10;
  tmp22 = year;
  tmp23 = days;
   printf( "year:%d day:%d\0A\00" , tmp22, tmp23);
  v1 = retval;
  goto LabelSkip;
LabelSkip:skip
}

proctype _main(chan __return) {
int retval;
int y1;
int y2;
int y3;
int i;
int tmp;
int conv;
bit cmp;
int tmp2;
int conv3;
int v0;
int call;
int tmp4;
int inc;
int currentLabel;
chan _syn = [0] of { int };
chan _return0 = [0] of { int };

Label0:
  currentLabel = 0;
  retval = 0;
  i = 365;
  goto Label1;

Label1:
  currentLabel = 1;
  tmp = i;
  conv =  tmp;
  cmp = (conv < 368);
  if
    ::(cmp!= 0) -> goto Label2
    ::(cmp==0) -> goto Label4
  fi;

Label2:
  currentLabel = 2;
  tmp2 = i;
  conv3 =  tmp2;
  v0 =  conv3;
  run _year(v0, _return0);
  goto Label3;

Label3:
  currentLabel = 3;
  tmp4 = i;
  inc = tmp4 + 1;
  i = inc;
  goto Label1;

Label4:
  currentLabel = 4;
  __return!0;
  goto LabelSkip;
LabelSkip:skip
}
init {
chan _syn = [0] of { int };
  run _main(_syn);
}


ltl p1 { [] ((_year@Label0) -> (<>(_year@Label10))) }
