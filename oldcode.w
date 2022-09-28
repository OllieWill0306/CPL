	int maxSteps = 0;
	int atNumber = 0;
	int i = 0;
	while i < 100{
		int testNumber = 1;
		while testNumber < 100000 {
			int num = testNumber;
			int steps = 0;
			while num != 4 {
				int isOdd = num % 2;
				if isOdd == 1 {
					num = (num * 3) + 1;
				}
				if isOdd == 0 {
					num = num / 2;
				}
				steps = steps + 1;
			}
			if steps > maxSteps {
				maxSteps = steps;
				atNumber = testNumber;
			}
			testNumber = testNumber + 1;
		}
		i = i + 1;
	}
	prints("maxSteps");
	printn(maxSteps);
	prints("atNumber");
	printn(atNumber);