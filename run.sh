#!/bin/bash

prompt_list=$(ls "$1")


if [[ "$1" == *"Prompts/ZeroShot"* ]] 
then
    prompt_list=$(ls "$1")
    echo "Executing Prompts from directory '$1'"
    for prompt in $prompt_list; do
        file="$1/$prompt"
        python3 Corrector_ZeroShot.py $file
        echo " === DONE : $file === "
    done

elif [[ "$1" == *"Prompts/OneExample"* ]] 
then
    prompt_list=$(ls "$1")
    echo "Executing Prompts from directory '$1'"
    for prompt in $prompt_list; do
        file="$1/$prompt"
        python3 Corrector_OneExample.py $file
        echo " === DONE : $file === "
    done

elif [[ "$1" == *"Prompts/MoreExamples"* ]] 
then
    prompt_list=$(ls "$1")
    echo "Executing Prompts from directory '$1'"
    for prompt in $prompt_list; do
        file="$1/$prompt"
        python3 Corrector_TwoExample.py $file
        echo " === DONE : $file === "
    done

else
    echo $'Incorrect path provided. Possible Paths :\n
    > "Prompts/ZeroShot"\n
    > "Prompts/ZeroShotCoT"\n
    > "Prompts/OneExample/FewShot"\n
    > "Prompts/OneExample/FewShotCoT"\n
    > "Prompts/MoreExamples/FewShot"\n
    > "Prompts/MoreExamples/FewShotCoT"\n'

fi

