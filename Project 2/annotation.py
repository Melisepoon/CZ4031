from graphviz import Digraph

class Annotator:

    def __init__(self):
        self.iCount = 0
        self.stepCount = 0
        self.dot = Digraph()
        self.dotCounter = 1
        self.totalCost = 0

    # wrapper function to do preprocessing on the qep and to only return finished string
    def wrapper(self, qep):
        # just to make it a bit nicer
        final = self.annotate(qep[0][0][0]['Plan'], True)[1]
        final = final[:-3]
        final += " to get the final result."
        print(self.dot.source)
        self.dot.format = 'png'  
        self.dot.node_attr={
            "shape":"rectangle",
            "width":"2",
            "height":"0.1"
            
        }
        image = self.dot.render('Graph')
        return final, image, self.totalCost

    def annotate(self, query, first = False):
        
        # for storing previous tables since they are not included in the qep
        joinTables = []
        child = ""

        # result string to be combined with current iter's output and returned
        result = ""

        self.totalCost += query["Total Cost"]
        
        nodeName = "N" + str(self.dotCounter)
        self.dot.node(nodeName, query["Node Type"])
        self.dotCounter += 1

        if "Plans" in query:
            for plan in query["Plans"]:
                temp = self.annotate(plan)
                joinTables.append(temp[0])
                result += temp[1]
                child = temp[2]
                # edgeName = nodeName + child
                self.dot.edge(nodeName , child)
                

        self.stepCount += 1
        result += "Step {}: ".format(self.stepCount)

        if query["Node Type"] == 'Seq Scan':
            table = query["Relation Name"]
            name = query["Alias"]
            step = "Perform sequential scan on table {} as {}".format(table, name)
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            step += ". \n"
            return table, result + step, nodeName


        elif query["Node Type"] == 'Index Scan':
            table = query["Relation Name"]
            name = query["Alias"]
            step = "Perform index scan on table {} as {} using index on {}".format(table, name, query["Index Name"])
            if "Index Cond" in query:
                step += " where {}".format(query["Index Cond"])
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            step += ". \n"
            return table, result + step, nodeName


        elif query["Node Type"] == 'Index-Only Scan':
            table = query["Relation Name"]
            name = query["Alias"]
            step = "Perform index scan on table {} as {} using index on {}".format(table, name, query["Index Name"])
            if "Index Cond" in query:
                step += " where {}".format(query["Index Cond"])
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            step += ". \n"
            return table, result + step, nodeName


        elif query["Node Type"] == 'CTE Scan':
            table = query["CTE Name"]
            name = query["Alias"]
            step = "Perform CTE scan on table {} as {}".format(table, name)
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            step += ". \n"
            return table, result + step, nodeName


        elif query["Node Type"] == 'Foreign Scan':
            table = query["Relation Name"]
            name = query["Alias"]
            step = "Perform foreign scan on table {} from schema {} as {}. \n".format(table, query["Schema"], name)
            return table, result + step, nodeName

        
        elif query["Node Type"] == 'Function Scan':
            table = query["Schema"]
            name = query["alias"]
            step = "Perform function {} on schema {} and return the results as {}".format(query["Function Name"], table, name)
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            step += ". \n"
            return table, result + step, nodeName

        
        elif query["Node Type"] == 'Subquery Scan':
            step = "The subquery results from the previous operation is read"
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            step += ". \n"
            return joinTables[0], result + step, nodeName

        
        elif query["Node Type"] == 'TID Scan':
            table = query["Relation"]
            name = query["Alias"]
            step = "Perform a Tuple ID scan on table {} as {}. \n".format(table ,name)
            return table, result + step, nodeName

        
        elif query["Node Type"] == 'Nested Loop':
            self.iCount += 1
            step = "Perform a nested loop join on tables {} and {}".format(joinTables[0], joinTables[1])
            if "Join Filter" in query:
                step += " under the condition {}".format(joinTables[0], joinTables[1], query["Join Filter"])
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName

        
        elif query["Node Type"] == 'Hash Join':
            self.iCount += 1
            step = "Perform a hash join on tables {} and {}".format(joinTables[0], joinTables[1])
            if "Hash Cond" in query:
                step += " under the condition {}".format(query["Hash Cond"])
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName


        elif query["Node Type"] == 'Merge Join':
            self.iCount += 1
            step = "Perform a merge join on tables {} and {}".format(joinTables[0], joinTables[1])
            if "Merge Cond" in query:
                step += " under the condition {}".format(query["Merge Cond"])
            if "Filter" in query:
                step += " with filter {}".format(query["Filter"])
            step += ". \n"
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName

        
        elif query["Node Type"] == 'Aggregate':
            self.iCount += 1
            step = "Perform aggregate on table {}".format(joinTables[0])
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName

        
        elif query["Node Type"] == 'Append':
            self.iCount += 1
            step = "Append the results from table {} to table {}".format(joinTables[0], joinTables[1]) 
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName

        
        elif query["Node Type"] == 'Gather':
            self.iCount += 1
            step = ("Perform gather on table {}".format(joinTables[0]))
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName

        
        elif query["Node Type"] == 'Gather Merge':
            step = "The results of the previous operation are gathered and merged. \n"
            return joinTables[0], result + step, nodeName

        
        elif query["Node Type"] == 'GroupAggregate':
            self.iCount += 1
            step = "Perform a group aggregate on table {}".format(joinTables[0])
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName

        
        elif query["Node Type"] == 'Hash':
            step = "Perform hashing on table {}. \n".format(joinTables[0])
            return joinTables[0], result + step, nodeName


        elif query["Node Type"] == 'HashAggregate':
            self.iCount += 1
            step = "Perform a hash aggregate on table {}".format(joinTables[0])
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName


        elif query["Node Type"] == 'Incremental Sort':
            step = "An incremetal sort is performed on table {} with sort key {}. \n".format(joinTables[0], query["Sort Key"])
            return joinTables[0], result + step, nodeName


        elif query["Node Type"] == 'Limit':
            step = "The specified number of rows is selected from table {}. \n".format(joinTables[0])
            return joinTables[0], result + step, nodeName


        elif query["Node Type"] == 'Materialize':
            step = "Materialize table {}. \n".format(joinTables[0])
            return joinTables[0], result + step, nodeName


        elif query["Node Type"] == 'MergeAppend':
            self.iCount += 1
            step = "Results from table {} are appended to table {}".format(joinTables[0], joinTables[1])
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName


        elif query["Node Type"] == 'ModifyTable':
            table = query["Relation Name"]
            step = "Table {} is modified. \n ".format(table)
            return table, result + step, nodeName


        elif query["Node Type"] == 'SetOp':
            self.iCount += 1
            step = "A set operation is performed on table {}".format(joinTables[0])
            if not first:
                step += " to get intermediate table T{}. \n".format(self.iCount)
            else: step += ". \n"
            return "T" + str(self.iCount), result + step, nodeName


        elif query["Node Type"] == 'Sort':
            step = "Perform a sort on table {} with sort key {}. \n".format(joinTables[0], query["Sort Key"])
            return joinTables[0], result + step, nodeName


        elif query["Node Type"] == 'Unique':
            table = query["Subplan Name"] if "Subplan Name" in query else joinTables[0]
            step = "Duplicates are removed from table {}. \n".format(table)
            return table, result + step, nodeName


        else:
            step = "Perform {}. \n".format(query["Node Type"])
            return joinTables[0], result + step, nodeName