## Code

For contributing and adding your code is actually pretty easy, since we use Github for our project.
Getting an account is a must if you want to contribute to [Dotto](https://github.com/LibreSprite/Dotto).

### Setting Up

#### Console
The following steps will show you the way how to setup your fork.

1. Fork [Dotto](https://github.com/LibreSprite/Dotto) so you have a personal repository to push to.

2. Clone your personal repository with `git clone --recursive https://github.com/YOUR-USERNAME/Dotto`. Remember to change the URL to your repository. The --recursive is used to download the third party libraries for building.

3. Move into the directory, on Linux this is done with `cd Dotto`.

4. Follow the instructions to create a build and make sure your local copy is working.

Add an upstream remote so you can get other developers' updates with `git remote add upstream https://github.com/LibreSprite/Dotto`.

#### Github Desktop

1. Go to the top-left of https://github.com/LibreSprite/Dotto 
2. Fork the project and go to your profile
3. Go to the forked repository and under code press Open with Github Desktop then set a location for it

### Updating Repository (Console)

These instructions will update both your local repository and online fork:

1. `git fetch upstream` Will download any new changes from the official LibreSprite repository.

2. `git checkout master` Will switch to your master branch.

3. `git merge upstream/master` Will merge or fast-forward your local master branch so it contains all the updates.

4. `git push origin master` Will update your online repository's master branch, it's a good idea to keep it up to date.

### Making Changes

So you have added a feature or fixed some bugs which were found by you, good job! You can now contribute to the master project. In that case you will need to create a feature branch.
This means that you will add your code to the master project and ensure that the master branch is working fine. With that said you might need to add comments in your code a clear title or a description on what you have found / created
so developers will know what you've been up to. This can be done by commenting or creating an issue or pull request here: https://github.com/LibreSprite/Dotto/pulls.


## Console
1. Call `git checkout master`, always make sure you are on master before making a feature branch.

2. Call `git checkout -b name-of-your-feature` This will create a feature branch and switch to it. Try to be specific, this helps developers by tracking it down in the future if we need to.

3. Make a meaningful change, you don't want to implement the whole feature in one shot generally. Try to break your task into meaningful (and revertible) chunks, though sometimes one chunk is all it takes like with small bug fixes.

4. To create a commit, start by verifying with `git status` that only the files you wanted to change are affected. If there are unexpected changes, please resolve them before continuing.

5. Stage all of your changes with `git add -A`.

6. Create your commit with `git commit -m "Type a precise description of only the changes made with this commit."` Try to describe the changes like it's a change log (hint, it is). Messages like "Convert 'bool' members in ui::Widget to flags" are what we're after to help developers.

7. Repeat steps 2-5 until the feature is complete.

## Github Desktop

1. Publish the branch and commit to your fork.
2. Go to https://github.com/LibreSprite/Dotto/pulls and create a new pull request.
3. Give a detailed title and description of your changes so that developers will know what you changed.
4. Wait for a response.
5. If you are asked to change something, change it and repeat step 1.


### Pushing Changes

You are ready for your contribution to be reviewed and merged. There are a few things you can do to make it easier for maintainers to review your code, so please follow all steps.

1. Follow the instructions for Updating Your Repository.

2. Switch back to your feature branch with `git checkout name-of-your-feature`.

3. Rebase your branch with `git rebase master`, this will make sure there are no conflicts with other people's contributions.

4. Rebuild your project to make sure everything still works. If things are now broken, resolve them, making commits along the way. Once resolved, return to step 1 as more changes could have been made in the meantime.

5. Push your branch to your online fork with `git push origin name-of-your-feature`.

6. On GitHub, create a pull request for your feature branch.

