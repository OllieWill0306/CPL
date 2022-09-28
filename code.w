fn void main() {
	int v1 = 0;
	int v2 = 1;
	int count = 0;
	while count < 10{
		int oldV2 = v2;
		v2 = fib(v1, v2);
		v1 = oldV2;
		printn(v1);
	}
	prints("Done");
}

fn int fib(int v1, int v2) {
	int ret = v1 + v2;
	return ret;
}