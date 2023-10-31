import os
import sys
import time
import openai


openai.api_key = os.getenv("OPENAI_API_KEY")


def format_prompt(f_path):
    
    conditions =["===== system =====","===== user ====="]
    
    with open(f_path, 'r') as file:
    #        lines = [line.strip() for line in file]
        content = file.read() 

    #content = '\\n'.join(lines)
    
    inital_pos = content.find(conditions[0]) + len(conditions[0])
    pos_user = content.find(conditions[1])
    user_read = pos_user+len(conditions[1])

    return content[inital_pos:pos_user],content[user_read:]

if __name__ == '__main__':

# call_API FICHEIRO_COM_PROMPT.txt
  cwd = os.getcwd() 
  prompt_path = os.path.join(cwd, sys.argv[1])

  system_part, user_part = format_prompt(prompt_path)

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
        "content": user_part
      }
    ],
    temperature=0,
    max_tokens=1500,
    top_p=1.0,
    frequency_penalty=0.0,
    presence_penalty=0.0
  )
  
  print(" === Time Taken for ", prompt_path,": %s seconds ==="% (time.time()-start_time))

  output_path = sys.argv[1].replace(".c",".txt")
  output_path = output_path.replace("Prompts","Results")
  
  
  with open(output_path,"w") as output:
        output.write(response.choices[0].message.content)