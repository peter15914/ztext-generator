
--[[
gen_procs__gen_sentences
	const wchar_t *file_name, int sent_cnt, int sent_len_min, int sent_len_max
--]]

------------------------------------------------------------------------------------------
local clock = os.clock
function sleep(n)  -- seconds
  LogImp("sleep " .. n);
  local t0 = clock()
  while clock() - t0 <= n do end
end


------------------------------------------------------------------------------------------
function gen_normal()
	sleep(30);
	gen_procs__gen_sentences("__res2/res_<rand_part>.txt", 30000, 20, 30);
end


------------------------------------------------------------------------------------------
function gen_with_kwds()
	gen_procs__gen_with_first_pair("хорошая", "игра", "", "<keyword>");
	gen_procs__gen_sentences("__res_kwd/res_<rand_part>.txt", 30000, 20, 30);
end


------------------------------------------------------------------------------------------
function main()
	LogImp("---lua work start");

	gen_normal();
	--gen_with_kwds();

	gen_procs__show_w3db_usage();

	LogImp("---lua work finish");
end

