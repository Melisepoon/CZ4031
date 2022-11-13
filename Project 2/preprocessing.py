import psycopg2 # need install

# allow use of with syntax
class DatabaseCursor():
    def __init__(self, settings):
        self.settings = settings

    def connectDatabase(self, config):
        self.conn = psycopg2.connect(
            host=config["host"],
            dbname=config["dbname"],
            user=config["user"],
            password=config["pwd"],
            port=config["port"]
        )   
        self.cur = self.conn.cursor()
        return self.cur

    def __exit__(self, exc_type, exc_val, exc_tb):
        # some logic to commit/rollback
        self.conn.close()

    def queryPlan(self, config, query):
        plans = []
        cursor = self.connectDatabase(config)
        for setting in self.settings:
            for i in setting:
                setting_string = f'SET {i[0]} to {i[1]}'
                cursor.execute(setting_string)
            cursor.execute("EXPLAIN (FORMAT JSON, SETTINGS ON) " + query)
            plan = cursor.fetchall()
            plans.append(plan)
        return plans

    def updateSchema(self, config):
        cursor = self.connectDatabase(config)
        query = "SELECT table_name, column_name, data_type, character_maximum_length as length FROM information_schema.columns WHERE table_schema='public' ORDER BY table_name, ordinal_position"
        cursor.execute(query)
        response = cursor.fetchall()

        # parse response as dictionary 
        schema = {}
        for item in response:
            # cols are table_name, column_name, data_type, length (nullable)
            attrs = schema.get(item[0], [])
            attrs.append(item[1])
            schema[item[0]] = attrs
        # log schema
        print("Database schema as follow: ")
        for t, table in enumerate(schema):
            print(t+1, table, schema.get(table))
        return schema