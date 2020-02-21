set CWD=%~dp0

set TTL_EXE="C:\Program Files (x86)\teraterm\ttpmacro.exe"

set TTL_FILE1="%CWD%\ttl\other\lteapi_test_api_state_dton.ttl"
set TTL_FILE2="%CWD%\ttl\other\lteapi_test_api_state_atch(1160_only).ttl"
set TTL_FILE3="%CWD%\ttl\other\lteapi_test_api_state_pwron.ttl"
set TTL_FILE4="%CWD%\ttl\other\lteapi_test_api_strpntst_pwron.ttl"
set TTL_FILE5="%CWD%\ttl\other\lteapi_test_api_strpntst_atch(1160_only).ttl"
set TTL_FILE6="%CWD%\ttl\other\lteapi_test_cb_edrx.ttl"
set TTL_FILE7="%CWD%\ttl\other\lteapi_test_cb_psm.ttl"
set TTL_FILE8="%CWD%\ttl\other\lteapi_test_api_strplcltm.ttl"
set TTL_FILE9="%CWD%\ttl\other\lteapi_test_cb_waitpin.ttl"

%TTL_EXE% %TTL_FILE1%
rem %TTL_EXE% %TTL_FILE2%
%TTL_EXE% %TTL_FILE3%
%TTL_EXE% %TTL_FILE4%
rem %TTL_EXE% %TTL_FILE5%
%TTL_EXE% %TTL_FILE6%
%TTL_EXE% %TTL_FILE7%
%TTL_EXE% %TTL_FILE8%
%TTL_EXE% %TTL_FILE9%

pause
