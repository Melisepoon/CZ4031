# Assignment 2

## Objective
* Design and implement an efficient algorithm that produces a description of the Query Execution Plan (QEP) based on an input query



## Submission files

* `interface.py`, `main.ui`: code and layout for the GUI
* `annotation.py`: code for generating the annotations
* `project.py`: main file that invokes all the necessary procedures 



## Setting up project

* Create a `config.json` file and place it in same directory as `project.py`

  ```json
  {
  	"TPC-H": {
  		"host": "localhost",
  		"dbname": "db4031",
  		"user" : "database_username",
  		"pwd" : "database_password",
  		"port" : "5432"
  	}
  }
  ```

* Create a virtual environment with Anaconda (optional)
  ```shell
  conda create --name cz4031a2 python=3.9.6
  conda activate cz4031a2
  ```
  
* Install required frameworks and libraries
  ```shell
  cd <project path>
  pip install -r requirements.txt
  ```



## Executing the program

* Ensure Python 3 is installed

* Ensure [PostgreSQL][2] database is hosted

* Execute the program with following command

  ```shell
  python project.py
  ```

## Dataset

* [TPC-H ][1] - v3.0.1 is used in the project



## Tools

* [PostgreSQL 15][2]
* [pgAdmin][3]
* [PyQt5][4]




[1]:http://www.tpc.org/tpc_documents_current_versions/current_specifications5.asp
[2]:https://www.postgresql.org/
[3]:https://www.pgadmin.org/
[4]:https://riverbankcomputing.com/software/pyqt/