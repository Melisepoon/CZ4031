import psycopg2
import queue
from configparser import ConfigParser

class Database:
    def __init__(self, host="localhost", port=5432, database="TPC-H", user="postgres", password="database"):
        self.connect = psycopg2.connect(host=host, port=port, database=database, user=user, password=password)
        self.cursor = self.conn.cursor()

    def execute(self, query):
        self.cursor.execute(query)
        query_results = self.cursor.fetchall()
        return query_results

    def close(self):
        self.cursor.close()
        self.connect.close()

def config(filename='database.ini', section='postgresql'):
    parser = ConfigParser()
    parser.read(filename)

    database = {}
    if parser.has_section(section):
        params = parser.items(section)
        for parameter in params:
            database[parameter[0]] = parameter[1]
    else:
        raise Exception('Section {0} not found in the {1} file'.format(section, filename,))
    
    return database

def connect():
    conn = None
    try:
        parameters = config()

        print('connecting to postgresql database')
        connect = psycopg2.connect(**parameters)

        cursor = conn.cursor()

        print('postgresql database version:')
        cursor.execute('SELECT version()')

        db_version = cursor.fetchone()
        print(db_version)

        cursor.close()
    except (Exception, psycopg2.DatabaseError) as error:
        print(error)
    finally:
        if connect is not None:
            connect.close()
            print('Database connection closed')

class Node(object):
    def __init__(self, description, nodeType, schema, relationName, 
                alias, children, groupKey, sortKey, joinType, indexName, 
                hashCondition, tableFilter, indexCondition, mergeCondition, 
                recheckCond, joinFilter, subPlanName, rows, time):
        self.description = description
        self.nodeType = nodeType
        self.schema = schema
        self.relationName = relationName
        self.alias = alias
        self.children = []
        self.groupKey = groupKey
        self.sortKey = sortKey
        self.joinType = joinType
        self.indexName = indexName
        self.hashCondition = hashCondition
        self.tableFilter = tableFilter
        self.indexCondition = indexCondition
        self.mergeCondition = mergeCondition
        self.recheckCond = recheckCond
        self.joinFilter = joinFilter
        self.subPlanName = subPlanName
        self.rows = rows
        self.time = time

    def append_children(self, child):
        self.children.append(child)
    
    def output_QEP_name(self, output_name):
        if "T" == output_name[0] and output_name[1:].isdigit():
            self.output_name = int(output_name[1:])
        else:
            self.output_name = output_name
    
    def set_step(self, step):
        self.step = step
    
    def update_description(self, description):
        self.description = description