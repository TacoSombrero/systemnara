
int build_target(makefile *make, const char *target, bool force_compile, bool suppress)
{
	// Check prereq and compile them if needed.
	rule *rule = makefile_rule(make, target);
	if (rule == NULL)
	{
		// There are no rule for this prerequirement.
		return 0;
	}
	const char **prereq = rule_prereq(rule);


	bool compile_target = false;
	// Compile prereq. If prereq is newer than the target.

	for (int i = 0; prereq[i] != NULL; i++)
	{
		
		if(build_target(make, prereq[i], force_compile, suppress) == 0){
			if (force_compile || is_modified_more_recently(prereq[i], target))
			{
				compile_target = true;
			}else{
				continue;
			}
		}
		//fprintf(stderr, "Compiling %s with prereq %s\n", target, prereq[i]);
		
	}

	if(force_compile || compile_target){
		// Compile the prereq.
		char **prereq_cmd = rule_cmd(makefile_rule(make, target));
		compile(suppress, prereq_cmd);
	}
	return 0;
}