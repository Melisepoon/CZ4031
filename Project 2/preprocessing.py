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