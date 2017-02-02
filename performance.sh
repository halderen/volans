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

cat schema_mysql.sql | mysql --user=opendnssec --password=test ods
mysql --user=opendnssec --password=test ods <<-END
load data local infile 'policies.dat' into table policies;
load data local infile 'keyclasses.dat' into table keyclasses;
load data local infile 'keyinstances.dat' into table keyinstances;
load data local infile 'zones.dat' into table zones;
END
echo "done"
exit 0
 
