#!/bin/bash

for i in {1..30}; do
	# Start the timer
	start_time=$(date +%s.%5N)

	./mdu_competition /usr/lib -j $i

	# End the timer
	end_time=$(date +%s.%5N)

	# Calculate the execution time
	execution_time=$(echo "$end_time - $start_time" | bc)

	# Format the output to show the time in seconds with three decimal places
	printf "$i, %.5f\n" "$execution_time"
done