import os
import sys
import time
import openai


openai.api_key = os.getenv("OPENAI_API_KEY")

def format_prompt(f_path):
    
  conditions =["===== system =====","===== user =====","## Example", "## Correction","===== assistant =====","### INCORRECT PERSISTENT MEMORY PROGRAM"]
  
  with open(f_path, 'r') as file:
  #        lines = [line.strip() for line in file]
    content = file.read() 
  #content = '\\n'.join(lines)
  
  inital_pos = content.find(conditions[0]) + len(conditions[0])

  pos_user = content.find(conditions[1])
  assistant_pos = content.find(conditions[4]) 
  
  example_pos = content.find(conditions[2])
  
  correction_pos = content.find(conditions[3])
  second_user = content.find(conditions[1],correction_pos)
  bug_pos = content.find(conditions[5])

      # return system, example_user, correction_assistant, bug_user
  return content[inital_pos:pos_user],content[example_pos:assistant_pos],content[correction_pos:second_user],content[bug_pos:]

if __name__ == '__main__':


  
# call_API FICHEIRO_COM_PROMPT.txt
  cwd = os.getcwd() 
  prompt_path = os.path.join(cwd, sys.argv[1])

  system_part, example_part, correction_part, bug_part = format_prompt(prompt_path)

  """
  print("\n===== system ======\n")
  print(system_part)
  print("\n===== example ======\n")
  print(example_part)
  print("\n===== correction/explanation ======\n")
  print(correction_part)
  print("\n===== bug ======\n")
  print(bug_part)
  """

  start_time = time.time()

  response = openai.ChatCompletion.create(
    model="gpt-3.5-turbo-0613",
    messages=[
      {
        "role": "system",
        "content": system_part
      },
      {
        "role": "user",
        "content": example_part
      },
      {
        "role": "assistant",
        "content": correction_part
      },
      {
        "role": "user",
        "content": bug_part
      }
    ],
    temperature=0,
    max_tokens=1296,
    top_p=1.0,
    frequency_penalty=0.0,
    presence_penalty=0.0
  )
  print(" === Time Taken for ", prompt_path,": %s seconds ==="% (time.time()-start_time))
  
  output_path = sys.argv[1].replace(".c",".txt")
  output_path = output_path.replace("Prompts","Results")
  

  with open(output_path,"w") as output:
        output.write(response.choices[0].message.content)