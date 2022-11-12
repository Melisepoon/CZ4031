import psycopg2
import queue
from configparser import ConfigParser

class Database:
    def __init__(self, host="localhost", port=5432, database="TPC-H", user="postgres", password="database"):
        self.conn = psycopg2.connect(host=host, port=port, database=database)
        self.cur = self.conn.cursor()

    def execute(self, query):
        self.cur.execute(query)
        query_results = self.cur.fetchall()
        return query_results

    def close(self):
        self.cur.close()
        self.conn.close()

def config(filename='database.ini', section='postgresql'):
    parser = ConfigParser()
    parser.read(filename)

    db = {}
    if parser.has_section(section):
        params = parser.items(section)
        for param in params:
            db[param[0]] = param[1]
    else:
        raise Exception('Section {0} not found in the {1} file'.format(section, filename,))
    
    return db

def connect():
    conn = None
    try:
        params = config()

        print('connecting to postgresql database')
        conn = psycopg2.connect(**params)

        cur = conn.cursor()

        print('postgresql database version:')
        cur.execute('SELECT version()')

        db_version = cur.fetchone()
        print(db_version)

        cur.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if conn is not None:
            conn.close()
            print('Database connection clsoed')