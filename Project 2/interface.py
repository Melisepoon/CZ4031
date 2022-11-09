import tkinter as tk
import tkinter.ttk as ttk
from tkinter import filedialog, Text
from tkinter import scrolledtext
import os

#base
root = tk.Tk()
root.geometry('1500x800')

greeting = tk.Label(text="Suggested Query Execution Plans and Representative AQPs")
greeting.pack()

#turquoise portion
canvas = tk.Canvas(root, height = 800, width=1500, bg= "#263D42")
canvas.pack()

#store the query here!
query_var = tk.StringVar()
def execute():
    query = query_var.get()
    query_var.set("")

query_label = ttk.Label(root,text="Enter your Query",font=('calibre',10,'bold'))
#create entry for input of query
query_entry = scrolledtext.ScrolledText(root,wrap=tk.WORD,width=40,height=7,font=('calibre',15))
#query_entry = tk.Entry(root,textvariable = query_var, font=('calibre',10,'normal'),width=100,height=100)
execute = tk.Button(root,text="Execute",fg="black",bg="white",width=12,height=2,command=execute)
execute.place(x=360,y=270)
query_label.place(x=60,y=100)
query_entry.place(x=180,y=100)

cost_label = ttk.Label(root,text="Query execution plan",font=('calibre',10,'bold'))
cost_label.place(x=60,y=400)

tree_label = ttk.Label(root,text="Natural language annotations",font=('calibre',10,'bold'))
tree_label.place(x=700,y=100)

#to place the annotations
#should we do tree form??
frame = tk.Frame(root,bg="white")
frame.place(relwidth=0.3,relheight=0.5,relx=0.6,rely=0.12)

root.mainloop() #to run app