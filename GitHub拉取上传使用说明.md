第一次拉取:`git clone git@github.com:xxx/xxx.git` 前面写用户名，后面写仓库名称

拉取之后会把`github`上的文件夹拉取到本地，在文件夹当中会有一个`.git/`隐藏文件夹，不要动它，这是自动生成的配置



- 后续的拉取(我自己写代码一般用不上)：

  `git pull`

- 后续提交按照顺序：

1. `git add .` 
  将所有的文件放到暂存区
  这个时候可以用 `git status` 查看文件状态，已经放到暂存区

2. `git commit -m "xxx"`
  使用git commit 提交文件，此时提交的修改仍然存储在本地，并没有上传到远程服务器。`-m` 后为此次提交的说明，解释做了哪些修改，方便他人理解。

3. `git push origin main`
  `origin`表示是远程仓库，`main`是我们操作的分支，我一般就用`main`，有的也可以用`master`分支
  我们可以用 `git remote -v` 查看地址



* 创建不同分支

  1.`git checkout --orphan xxx< new branch name>`// 创建一个与主分支无关联的新分支

  2.创建之后，若默认继承了主分支的树，这时需要清楚索引和工作树

  ​	`git rm -rf . // 注意后面的点`

  3.执行完毕，将新分支的项目copy到仓库文件夹下，执行推送命令即可，注意后面更换分支名称

  这里假设新分支名称为：next-branch

  `git add .`
  `git commit -m 'next-branch commit'`
  `git push origin next-branch// 此时已经完成新分支next-branch的推送`

  
