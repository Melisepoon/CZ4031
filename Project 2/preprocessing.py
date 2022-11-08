import psycopg2
import queue
from configparser import ConfigParser

class database:
    def __init__(self, host="localhost", port=5432, database="TPC-H", user="postgres", password="database"):
        self.conn = psycopg2.connect(host=host, port=port, database=database)