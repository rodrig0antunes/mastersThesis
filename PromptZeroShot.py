import sys
import os

def create_prompt_Zero_Shot(bug,flag):
    """
    Creates Zero shot prompts to be feed to a LLM model. Receives a bug argument that has the path related to the bug wanted to be analysed by the model. 
    The flag argument serves to identify when the prompt should be a Chain of thought (value of argument is False) or not (value of argument is True).
    """
    code_to_correct = take_data(bug)
    
    if flag:
        cot = "" 
    else:
        cot = "Let's think step by step on how to resolve the bug."
    
    prompt = f"""
===== system =====

You are a helpful programming assistant and an expert in the development of Persistent Memory programs. You are helping a user repair the bugs inside a Persistent Memory program. 
The user has written a program in C programming language while using the PMDK library libpmemobj. However, the program has some bugs and is not working as expected. 
The user has analysed the program with a bug detection tool that has located the bug or bugs. You will use this information to generate a corrected version of the program.
The bug or bugs to repair will be located in an area of the code delimited by an expression. The beggining and end of the area of the code where a bug is and where the fix is 
supposed to go will be delimited by the exprexion '// BUG //'.
When presenting the correction, present the whole code and not just the corrected segment of the code.
Put the whole corrected program within code delimiters, as follows: 
                ''' C
                # YOUR CODE HERE
                '''.

===== user =====

### INCORRECT PERSISTENT MEMORY PROGRAM

''' C
{code_to_correct}
'''.
    {cot}
"""
    return prompt

def take_data(f_path):
    """
    TO BE COMMENTED
    """
    with open(f_path, 'r') as input:
        content = input.read()
    return content

def get_example(example_dir,btype):
    examples = []
    type_dir = os.path.join(cwd, example_dir,btype)
    for file in os.listdir(type_dir):
        if file.endswith('.txt'):
            examples.append(os.path.join(type_dir, file))
            # with open(os.path.join(type_dir, file)) as ex:
                
    return examples

def conditional_read(ex_path,flag):
    """ Reads the example input from the located in the file with path ex_path. The examples contains 3 parts : bad code, corrected code and explanation.
        Each part is delimited by an expression:\n
        ===== Q: ===== (bad code);\n
        ===== A: ===== (corrected code);\n 
        ===== E: ===== (explanation);\n
        Receives an argument flag that identifies the type of read.
        Flag = 0 : Read all the example. This is done for the CoT that requires the explanation;
        Flag = 0 : Read till explanation, which is identified by ===== E: =====. This is done for the non-CoT.
    """
    try:
        condition = "===== E: =====" 
        with open(ex_path, 'r') as file:
            content = file.read()
            if flag :
                pos = content.find(condition)
                return content[:pos]
            else:
                return content
            
    except FileNotFoundError:
        print(f"File '{ex_path}' not found.")

if __name__ == '__main__':
    
    cwd = os.getcwd() #current working directory
    examples_directory = os.path.join(cwd, "BugRepository","examples")
    
    # direcory that contains all of the bugs and examples for each type of bug
    
    bug_types = ["btree","ctree","hashmap_atomic","hashmap_tx","rbtree","real_bug"] # list of programs that contain bugs

    # Having a list of bug types that represent each sub directory, selecting each sub directory and then select 
    for btype in bug_types:
        bug_directory = os.path.join(cwd,"Repository","Bugs",btype)
        for file in os.listdir(bug_directory):
            if file.endswith('.c'):
                bug = os.path.join(bug_directory, file)
                prompt = create_prompt_Zero_Shot(bug,True)
                prompt_COT = create_prompt_Zero_Shot(bug,False)
                print("\n> DONE FILE: ", file)

                output_path = os.path.join(cwd,"Prompts","ZeroShot",file)            
                output_COT_path = os.path.join(cwd,"Prompts","ZeroShotCoT",file)             
                
                with open(output_path,"w") as output:
                    output.write(prompt)
                with open(output_COT_path,"w") as output:
                    output.write(prompt_COT)
            else: 
                continue
