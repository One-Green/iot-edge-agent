"""
Basic key-value storage system
# TODO: move to etcd once arm build stable

Author: Shanmugathas Vigneswaran
email: shanmugathas.vigneswaran@outlook.fr

"""

from peewee import *
from datetime import datetime

db = SqliteDatabase('database.sqlite')


class Buffer(Model):
    modified = DateTimeField()
    key = TextField()
    value = BlobField()

    class Meta:
        database = db


def get_db():
    db.connect()
    db.create_tables(
        [
            Buffer
        ]
    )
    return db


def db_update_or_create(key, value):
    try:
        Buffer.get(Buffer.key == key).get()
        Buffer.update(
            {Buffer.value: value,
             Buffer.modified: datetime.now()}
        ).where(
            Buffer.key == key
        ).execute()
    except Buffer.DoesNotExist:
        Buffer.create(key=key, value=value, modified=datetime.now())


def get_state():
    d = {}
    for _ in Buffer().select():
        d[_.key] = _.value
    return d
