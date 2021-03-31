from db import get_db, db_update_or_create

db = get_db()

db_update_or_create("test", 0.155555545646)
db_update_or_create("test1", 0.255555545646)
db_update_or_create("test2", 0.355555545646)
db_update_or_create("test3", 0.555555545646)
db_update_or_create("test4", 0.4555555545646)
db_update_or_create("test", 0.45455555545646)
