RELEASEDATE = $$system(powershell -Command "(Get-Date).ToString('yyyy-MM-dd')")
include(../mainconfig.pri)


# Define a custom target for create_xml
system($$quote(python "./substitute.py" "$$VERSION" "$$RELEASEDATE"))
