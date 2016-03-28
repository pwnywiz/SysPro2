# SysPro Project 2

Git Commands
-------------
// General -->
git status ,
git log

// Repository -->

// Add, commit and push the files to your branch
git add <file_name> ,
git commit -m "Description" ,
git push origin <branch_name> ,

// Pull your files from the github repository
git pull ,

// Clone (download and create) a github repository locally
git clone username@host:/path/to/repository ,

// Pull the latest github version regardless the local version
git fetch --all ,
git reset --hard origin/master ,

// Delete files added to git by mistake
git ls-files --deleted -z | xargs -0 git rm

// Branches -->
git branch ,
git checkout <branch_name> ,
git merge <branch_name>
