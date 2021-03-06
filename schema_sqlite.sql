PRAGMA journal_mode = WAL;
PRAGMA synchronous = OFF;

CREATE TABLE properties (
  propertyName VARCHAR(32),
  propertyValue VARCHAR(255)
);

INSERT INTO  properties VALUES ( 'version', 1 );

CREATE TABLE rrsets (
  revision  INTEGER NOT NULL DEFAULT 0,
  sigage    TIMESTAMP DEFAULT NULL,
  dname     VARCHAR(256) NOT NULL,
  dcontent  TEXT
);
CREATE INDEX IF NOT EXISTS rrsets_idx_revision ON rrsets ( revision, dname );
CREATE INDEX IF NOT EXISTS rrsets_idx_dname    ON rrsets ( dname );

CREATE TABLE refrrsets (
  dname    VARCHAR(256) NOT NULL,
  revision INTEGER NOT NULL
);
CREATE index IF NOT EXISTS refrrsets_idx_dname ON refrrsets ( dname );

CREATE VIEW currrsets AS SELECT MAX(revision) AS revision, dname AS dname
                         FROM rrsets
                         GROUP BY dname;
CREATE TRIGGER rrsetinsert INSERT ON rrsets
  BEGIN
    REPLACE INTO refrrsets VALUES ( NEW.dname, NEW.revision );
  END;
INSERT INTO refrrsets SELECT dname, max(revision) FROM rrsets GROUP BY dname;
