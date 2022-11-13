import sys 
import time
import json

from PyQt5.QtWidgets import QApplication # need install
from qt_material import apply_stylesheet #, list_themes # need install
from interface import *
from annotation import *
from preprocessing import *

# extract hard coded values
FILE_CONFIG = "config.json"
FILE_APP_THEME = "dark_blue.xml" #list_themes()[12]

settings = [

        [
            ["ENABLE_HASHJOIN","ON"],
            ["ENABLE_MERGEJOIN","ON"],
            ["ENABLE_SEQSCAN","ON"]
        ],

        [
            ["ENABLE_HASHJOIN","OFF"],
            ["ENABLE_SEQSCAN","OFF"]
        ],

        [
            ["ENABLE_MERGEJOIN","OFF"],
            ["ENABLE_SEQSCAN","OFF"]
        ]
    ]

class Program():
    def __init__(self):
        with open(FILE_CONFIG, "r") as file:
            self.config = json.load(file)
        # init ui components
        self.app = QApplication(sys.argv)
        apply_stylesheet(self.app, theme=FILE_APP_THEME)
        self.plans = []
        self.counter = 0
        self.window = UI()
        self.databaseCursor = DatabaseCursor(settings)
        self.window.setOnDatabaseChanged( lambda: self.onDatabaseChanged())
        self.window.setOnAnalyseClicked( lambda: self.analyseQuery() )
        self.window.setOnBackClicked( lambda: self.back() )
        self.window.setOnNextClicked( lambda: self.next() )

    def run(self):
        self.window.show()
        list_db = list(self.config.keys() )
        print(f"List of database configs from json file: {list_db}")
        self.window.setListDatabase(list_db)
        sys.exit(self.app.exec_())
        
    def onDatabaseChanged(self):
        # check cur database, update schema?
        cur_db = self.window.list_database.currentText()
        print(f"Current selected database is {cur_db}")
        self.db_config = self.config[cur_db]
        self.updateDatabase()

    def hasDbConfig(self):
        if not hasattr(self, "db_config"): 
            return False
        if self.db_config == None:
            return False
        return True
    
    def back(self):
        if self.plans:
            if self.counter == 1 or self.counter == 2:
                self.counter -= 1
                try:
                    result = Annotator().wrapper(self.plans[self.counter])
                    plan_annotated = result[0]
                    print("annotated qep: \n%s"%plan_annotated)
                    others = []
                    others.append(settings[self.counter])
                    others.append(str(int(result[2])))
                    self.window.setResult( plan_annotated, others )
                    image = result[1]
                    self.window.setImage(image)
                except Exception as e:
                    print(str(e))
                    self.window.showError("Unable to analyse query!", e)



    def next(self):
        if self.plans:
            if self.counter == 0 or self.counter == 1:
                self.counter += 1
                try:
                    result = Annotator().wrapper(self.plans[self.counter])
                    plan_annotated = result[0]
                    print("annotated qep: \n%s"%plan_annotated)
                    others = []
                    others.append(settings[self.counter])
                    others.append(str(int(result[2])))
                    self.window.setResult( plan_annotated, others )
                    image = result[1]
                    self.window.setImage(image)
                except Exception as e:
                    print(str(e))
                    self.window.showError("Unable to analyse query!", e)

    def analyseQuery(self):
        if not self.hasDbConfig():
            self.window.showError("Database configuration is not found")
            return
        try:
            query = self.window.readInput()
            if not query:
                print("query is empty")
                return
            print("query: \n%s"%query)

            self.plans = self.databaseCursor.queryPlan(self.db_config, query)
            self.counter = 0
            result = Annotator().wrapper(self.plans[self.counter])
            plan_annotated = result[0]
            print("annotated qep: \n%s"%plan_annotated)
            others = []
            others.append(settings[self.counter])
            others.append(str(int(result[2])))
            print(others)
            self.window.setResult( plan_annotated, others )
            image = result[1]
            self.window.setImage(image)
                
        except Exception as e:
            print(str(e))
            self.window.showError("Unable to analyse query!", e)

    def updateDatabase(self):
        if not self.hasDbConfig(): 
            self.window.setSchema(None)
            self.window.showError("Database configuration is not found")
            return
        try:
            schema = self.databaseCursor.updateSchema(self.db_config)
            self.window.setSchema(schema)

        except Exception as e:
            print(str(e))
            self.window.showError("Unable to retrieve schema information!", e)
    
    
if __name__ == "__main__":
    Program().run()