# if return code is 1, then test failed

# get all UCI engines
uci_engines=$(find ../archive -name "main*")

# make dir 'working_engines' if it doesn't exist
mkdir -p working_engines

# run test.py with all UCI engines
for engine in $uci_engines; do
	printf "Testing $engine: "
	# timeout 2s
	timeout 2s python3 test_uci.py $engine > /dev/null
	# fail on timeout
	if [ $? -ne 0 ]; then
		printf "FAIL\n"
	else
		printf "PASS\n"
		# copy working engine to working_engines
		cp $engine working_engines
	fi
done