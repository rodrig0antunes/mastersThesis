import sys
import os
import csv


def create_prompt_Few_Shot(bug,example_path,flag):
    """
    Note that few shot will provide all the bugs in the respective bug socket. This means that the number of shots depends on the number of examples
    present in the respective directory

    """

    # Get examples for the user section
    # -> examples = []
 
    if flag:
        explanation = ""
        space = ""
        example,correction = conditional_read(example_path,flag)
    else:
        space = "\n"        
        example,correction,explanation = conditional_read(example_path,flag)
    
    code_to_correct = take_data(bug) #input("\nEnter the code snippet to be corrected:\n")
    # Construct the Markdown prompt

    prompt = f"""
===== system =====

You are a helpful programming assistant and an expert in the development of Persistent Memory programs. You are helping a user repair the bugs inside a Persistent Memory program. 
The user has written a program in C programming language while using the PMDK library libpmemobj. However, the program has some bugs and is not working as expected. 
The user has analysed the program with a bug detection tool that has located the bug or bugs. You will use this information to generate a corrected version of the program.
The bug or bugs to repair will be located in an area of the code delimited by an expression. The beggining and end of the area of the code where a bug is and where the fix is 
supposed to go will be delimited by the expression '// BUG //'.
When presenting the correction, present the whole code and not just the corrected segment of the code.
Put the whole corrected program within code delimiters, as follows:
                ''' C
                # YOUR CODE HERE
                '''.

===== user =====

### EXAMPLES

## Example

''' C
{example}
'''.

===== assistant =====

## Correction

''' C
{correction}
'''.
{space}{explanation}
===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM

''' C
{code_to_correct}
'''.
"""
    return prompt

def take_data(f_path):
    with open(f_path, 'r') as input:
        content = input.read()
    return content

def get_example(ex_dict,key):
    """
    Function that allows the to get examples from a the dictionary 
    using the file name has key. Returns the path to the required example.
    """
    example = os.path.join(cwd,"Repository","Examples",ex_dict[key][0])             
    return example

def build_ex_dict(ex_path):
    """
    Builds a dictionary of the examples that are helful for each bug program in the context of
    few shot learning approach. 
    "ex_path" variable holds the path to the .csv file that contains this information    
    """
    example_dict = {}

    with open(ex_path,"r") as ex_csv : 
    
        line_reader = csv.reader(ex_csv,delimiter=";")
        for row in line_reader:
            example_dict[row[0]] = [row[1]+".txt",row[2]+".txt"]
    
    return example_dict


def conditional_read(ex_path,flag):
    """ Reads the example input from the located in the file with path ex_path. The examples contains 3 parts : bad code, corrected code and explanation.
        Each part is delimited by an expression:\n
        ===== Q: ===== (bad code);\n
        ===== A: ===== (corrected code);\n 
        ===== E: ===== (explanation);\n
        Receives an argument flag that identifies the type of read.
        Flag = 0 : Read all the example. This is done for the CoT that requires the explanation;
        Flag = 1 : Read till explanation, which is identified by ===== E: =====. This is done for the non-CoT.
        Output : [example, correction, explanation]
    """
    try:
        conditions =["## Example","## Correction", "## Explanation"] 
        with open(ex_path, 'r') as file:
            content = file.read()
            #example = content.find(conditions[0])
            correction = content.find(conditions[1])
            explanation = content.find(conditions[2])
            if flag :
                return content[len(conditions[0]):correction],content[correction+len(conditions[1]):explanation]
            else:
                return content[len(conditions[0]):correction],content[correction+len(conditions[1]):explanation],content[explanation:]
            
    except FileNotFoundError:
        print(f"File '{ex_path}' not found.")

if __name__ == '__main__':
    
    cwd = os.getcwd() #current working directory
    
    # direcory that contains all of the bugs and examples for each type of bug
    
    bug_types = ["btree","ctree","hashmap_atomic","hashmap_tx","rbtree","real_bug"] # list of programs that contain bugs
    csv_path = os.path.join(cwd,"ExampleDB.csv")

    example_dict = build_ex_dict(csv_path)
      
    for btype in bug_types:
        bug_directory = os.path.join(cwd,"Repository","Bugs",btype)
        for file in os.listdir(bug_directory):
            if file.endswith('.c'):
                example = get_example(example_dict, file)
                bug = os.path.join(bug_directory, file)
                prompt = create_prompt_Few_Shot(bug, example, 1)
                prompt_COT = create_prompt_Few_Shot(bug, example, 0)
                print("\n> DONE FILE: ", file)
                output_path = os.path.join(cwd,"Prompts","OneExample","FewShot",file)            
                output_COT_path = os.path.join(cwd,"Prompts","OneExample","FewShotCoT",file)            
                
                with open(output_path,"w") as output:
                    output.write(prompt)
                with open(output_COT_path,"w") as output:
                    output.write(prompt_COT)
            else: 
                continue