set CWD=%~dp0

set TTL_EXE="C:\Program Files (x86)\teraterm\ttpmacro.exe"

set TTL_FILE1="%CWD%\ttl\comb\lteapi_test_comb01.ttl"
set TTL_FILE2="%CWD%\ttl\comb\lteapi_test_comb02.ttl"
set TTL_FILE3="%CWD%\ttl\comb\lteapi_test_comb03.ttl"
set TTL_FILE4="%CWD%\ttl\comb\lteapi_test_comb04(1160_only).ttl"
set TTL_FILE5="%CWD%\ttl\comb\lteapi_test_comb05.ttl"
set TTL_FILE6="%CWD%\ttl\comb\lteapi_test_comb06.ttl"
set TTL_FILE7="%CWD%\ttl\comb\lteapi_test_comb07.ttl"
set TTL_FILE8="%CWD%\ttl\comb\lteapi_test_comb08(1160_only).ttl"
set TTL_FILE9="%CWD%\ttl\comb\lteapi_test_comb09(1160_only).ttl"
set TTL_FILE10="%CWD%\ttl\comb\lteapi_test_comb10.ttl"
set TTL_FILE11="%CWD%\ttl\comb\lteapi_test_comb11.ttl"
set TTL_FILE12="%CWD%\ttl\comb\lteapi_test_comb12(1160_only).ttl"
set TTL_FILE13="%CWD%\ttl\comb\lteapi_test_comb13.ttl"
set TTL_FILE14="%CWD%\ttl\comb\lteapi_test_comb15.ttl"

%TTL_EXE% %TTL_FILE1%
%TTL_EXE% %TTL_FILE2%
%TTL_EXE% %TTL_FILE3%
rem %TTL_EXE% %TTL_FILE4%
%TTL_EXE% %TTL_FILE5%
%TTL_EXE% %TTL_FILE6%
%TTL_EXE% %TTL_FILE7%
rem %TTL_EXE% %TTL_FILE8%
rem %TTL_EXE% %TTL_FILE9%
TTL_EXE% %TTL_FILE10%
%TTL_EXE% %TTL_FILE11%
rem %TTL_EXE% %TTL_FILE12%
%TTL_EXE% %TTL_FILE13%
%TTL_EXE% %TTL_FILE14%


pause
