drop table if exists properties;

create table if not exists properties (
  propertyName varchar(32),
  propertyValue varchar(32),
);
 
drop table if exists rrsets;
create table if not exists rrsets (
    revision bigint not null,
    name varchar(64) not null,
    content blob,
    primary key ( revision, name ),
    index ( revision ),
    index ( name )
);

load data local infile ()

SET @@session.unique_checks = 0;
SET @@session.foreign_key_checks = 0;
SET @@session.sql_log_bin = 0;
set unique_checks = 0;
set foreign_key_checks = 0;
set sql_log_bin=0;
ALTER TABLE rrsets DISABLE KEYS;
BEGIN;
LOAD DATA LOCAL INFILE '/var/tmp/data.out' IGNORE INTO TABLE rrsets;
COMMIT;
ALTER TABLE rrsets ENABLE KEYS;

drop table if exists rrsets;
create table if not exists rrsets (
    revision bigint not null,
    name varchar(64) not null,
    content varchar(64),
    primary key ( revision, name ),
    index ( revision ),
    index ( name )
);

initialize database
insert change
  change 