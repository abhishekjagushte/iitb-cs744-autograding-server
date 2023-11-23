nclient=$1
cat ./analysisFiles/plots/pref$nclient.txt | awk -v load="$nclient" '
	BEGIN{
		total=0
		sum=0
	}
	    
	{
		if ($15 ~ /^[0-9]+$/) {
			sum=sum+$15
			total=total+1
		}
	}

	END{
		printf("Average Utilization for (load level = %d) = %f\n", load, 100-(sum/total))
	}'