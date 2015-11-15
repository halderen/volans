#!/bin/sh

m=100       ;# number of policies
n=1000000   ;# number of zones
rm -f policies.dat keyclasses.dat keyinstances.dat zones.dat
i=0; while [ $i -lt $m ]; do
  echo >> policies.dat "acomplexpolicyname$i	A nice long description that noone will actually user describing a policy blalbla.  want a longer description, this is just a test."
  i=`expr $i + 1`
done
i=0; while [ $i -lt $m ]; do
  echo >> keyclasses.dat "`expr $i \* 4 + 0`	acomplexpolicyname$i	256	4	0	0"
  echo >> keyclasses.dat "`expr $i \* 4 + 1`	acomplexpolicyname$i	256	4	0	0"
  echo >> keyclasses.dat "`expr $i \* 4 + 2`	acomplexpolicyname$i	256	4	0	0"
  echo >> keyclasses.dat "`expr $i \* 4 + 3`	acomplexpolicyname$i	256	4	0	0"
  i=`expr $i + 1`
done
i=0; while [ $i -lt $n ]; do
  echo >> keyinstances.dat "`expr $i \* 4 + 0`	f0e1d2c3b4a5968778695a4b3c2d1e0f	thisisacomplexandtoolongzonenameifyoudlike$i	`expr $i \* 4 % $m + 0`	0	0	0	0	0	0	0	0	0	0"
  echo >> keyinstances.dat "`expr $i \* 4 + 1`	f0e1d2c3b4a5968778695a4b3c2d1e0f	thisisacomplexandtoolongzonenameifyoudlike$i	`expr $i \* 4 % $m + 1`	0	0	0	0	0	0	0	0	0	0"
  echo >> keyinstances.dat "`expr $i \* 4 + 2`	f0e1d2c3b4a5968778695a4b3c2d1e0f	thisisacomplexandtoolongzonenameifyoudlike$i	`expr $i \* 4 % $m + 2`	0	0	0	0	0	0	0	0	0	0"
  echo >> keyinstances.dat "`expr $i \* 4 + 3`	f0e1d2c3b4a5968778695a4b3c2d1e0f	thisisacomplexandtoolongzonenameifyoudlike$i	`expr $i \* 4 % $m + 3`	0	0	0	0	0	0	0	0	0	0"
  i=`expr $i + 1`
done
i=0; while [ $i -lt $n ]; do
  echo >> zones.dat "thisisacomplexandtoolongzonenameifyoudlike$i	ladidaladidaladidaladidaladidaladidaladidaladidaladidaladidaladidaladidaladida	3	ajslkahjdflkhajdfha;sdhfaf	4	aksjdfalsdjfalkjflakjdlkajsdlfjalkdjfalksjdlkfajsdakjf	0	1	2	0	0	0	0	0	0	0	0"
  i=`expr $i + 1`
done

mysql --user=opendnssec --password=iuap ods <<-END
drop table if exists zones;
drop table if exists keyinstances;
drop table if exists keyclasses;
drop table if exists policies;

create table if not exists zones (
  name varchar(64) primary key,
  policy varchar(64) key,
  label text not null,
  inadapttype integer,
  inadapturi text,
  outadapttype integer,
  outadapturi text,
  f1 integer,
  f2 integer,
  f3 integer,
  f4 integer,
  t1 timestamp,
  t2 timestamp,
  t3 timestamp,
  t4 timestamp,
  t5 timestamp,
  t6 timestamp,
  t7 timestamp,
  t8 timestamp
);
create table if not exists keyinstances (
  id integer primary key,
  locator text,
  zone varchar(64),
  keyclass integer,
  inception timestamp,
  state integer,
  dsstate integer,
  dsstatesince timestamp,
  ksstate integer,
  ksstatesince timestamp,
  rrsigstate integer,
  rrsigstatesince timestamp,
  kssigstate integer,
  kssigstatesince timestamp
);
create table if not exists keyclasses (
  id integer primary key,
  policy varchar(64) not null,
  nbit integer,
  algorithm integer,
  role integer,
  lifetime timestamp
);
create table if not exists policies (
  name varchar(64) primary key,
  label text not null
);
load data local infile 'policies.dat' into table policies;
load data local infile 'keyclasses.dat' into table keyclasses;
load data local infile 'keyinstances.dat' into table keyinstances;
load data local infile 'zones.dat' into table zones;
END
echo "done"
exit 0
