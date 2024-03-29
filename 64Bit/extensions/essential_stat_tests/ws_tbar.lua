local btnDescStat = std.Button.new{
	title = "Descriptive Stat", 
	py = ACTIVEDIR.."/frmDescriptiveStats.py", 
	img = ACTIVEDIR.."/icons/descriptivestat.jpg"}
	

local btnRandNum = std.Button.new{
	title = "Number Gen", 
	py = ACTIVEDIR.."/frmrandnumgener.py", 
	img = ACTIVEDIR.."/icons/randomnumgener.jpg"}

local btnZTest = std.Button.new{
	title = "Z-test", 
	py = ACTIVEDIR.."/frmtest_z.py", 
	img = ACTIVEDIR.."/icons/test_z.png"}

local btnFTest = std.Button.new{
	title = "F-test", 
	py = ACTIVEDIR.."/frmtest_f.py", 
	img = ACTIVEDIR.."/icons/test_f.png"}

local btnSignTest = std.Button.new{
	title = "Sign Test", 
	py = ACTIVEDIR.."/frmtest_sign.py", 
	img = ACTIVEDIR.."/icons/test_sign.png"}

local btnttest1 = std.Button.new {
	title = "1 sample t-test", 
	py = ACTIVEDIR.."/frmtestt_1sample.py", 
	img = ACTIVEDIR.."/icons/t_test1sample.png"}

local btnttest2 = std.Button.new {
	title = "2 sample t-test", 
	py = ACTIVEDIR.."/frmtestt_2sample.py", 
	img = ACTIVEDIR.."/icons/t_test2sample.png"}

btnttest_paired = std.Button.new{
	title = "Paired t-test", 
	py = ACTIVEDIR.."/frmtestt_paired.py", 
	img = ACTIVEDIR.."/icons/t_testpaired.png"}

local btnANOVA1 = std.Button.new{
	title = "1-way ANOVA", 
	py = ACTIVEDIR.."/frmanova_singlefactor.py", 
	img = ACTIVEDIR.."/icons/anovasinglefactor.png"}

local btnANOVA2 = std.Button.new{
	title = "2-way ANOVA", 
	py = ACTIVEDIR.."/frmanova_twofactor.py", 
	img = ACTIVEDIR.."/icons/anova2factor.png"}

local btnLinearRegres = std.Button.new{
	title = "Linear Regression", 
	py = ACTIVEDIR.."/frmregression_linear.py", 
	img = ACTIVEDIR.."/icons/regression.png"}


local btntest_t = std.HybridButton.new(btnttest2)

btntest_t:add(btnttest1)
btntest_t:add(btnttest_paired)



local pageStat = std.ToolBarPage.new("Statistics") 
pageStat:add(btnDescStat)
pageStat:add(btnRandNum)
pageStat:add(btnFTest)
pageStat:add(btnZTest)
pageStat:add(btntest_t)
pageStat:add(btnSignTest)
pageStat:add(btnANOVA1)
pageStat:add(btnANOVA2)
pageStat:add(btnLinearRegres)

pageStat:start()