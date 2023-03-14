# Solution Write Up for 'we_need_to_break_free'
# Expected Behavior
Give an example here of what you are given in the challenge / what it look like to run

![What the user sees](assets/images/01.png?raw=true "interface")

# Solution

1. Users should notice there is only 1 place for user input. The checksum POST button.
2. They can edit to fuzz the post parapeters. For example in Firefox,

	1. Open Web Developers tools
	2. Click the chesksum button
	3. Notice the POST item in the Network section
	4. If you right click on it you can "edit and resend"
3. Edit the POST Parameter 'computerHashDropDown' to have special characters.
4. They may encounter some errors during compilation. Note that these errors are linux errors. They should recognize this and write code to execute other linux commands.
5. This may be a path for them to follow:

#This shows a list of what appear to be a folder 'flag'
computeHash=checksum&computeHashDropDown=newmap.png;ls

#This shows that flag.txt is in this folder
computeHash=checksum&computeHashDropDown=newmap.png;ls%20flag

#This prints out the flag
computeHash=checksum&computeHashDropDown=newmap.png;cat%20flag/flag.txt

6. They can now see the flag in the 'response' tab.

ARC{Inconceivable!}

![solution](assets/images/02.png?raw=true "solution")


# Flag
[//]: <> (Add the flag below)
**ARC{Inconceivable!}**
