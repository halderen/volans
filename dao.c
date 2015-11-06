
C_GetFunctionList
14    sqlite3 *db;

22    rc = sqlite3_open(argv[1], &db);
23    if( rc ){
24      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
25      sqlite3_close(db);
26      return(1);
27    }
28    rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
29    if( rc!=SQLITE_OK ){
30      fprintf(stderr, "SQL error: %s\n", zErrMsg);
31      sqlite3_free(zErrMsg);
32    }
33    sqlite3_close(db);
34    return 0;
35  }
