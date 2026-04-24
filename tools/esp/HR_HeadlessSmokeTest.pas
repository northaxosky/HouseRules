{
  House Rules — FO4Edit headless smoke test.

  Purpose: Prove the CLI pipeline works end-to-end without touching any records.
  Writes a log file next to the FO4Edit install.

  Invoke:
    FO4Edit.exe -script:HR_HeadlessSmokeTest.pas -IKnowWhatImDoing Fallout4.esm
}
unit UserScript;

function Initialize: integer;
var
  sl: TStringList;
  masterCount, i: integer;
  logPath: string;
begin
  logPath := ProgramPath + 'HR_SmokeTest.log';
  sl := TStringList.Create;
  try
    sl.Add('=== House Rules FO4Edit smoke test ===');
    sl.Add('Timestamp: ' + DateTimeToStr(Now));
    sl.Add('Program path: ' + ProgramPath);
    sl.Add('Data path: ' + DataPath);

    masterCount := FileCount;
    sl.Add('Loaded plugin count: ' + IntToStr(masterCount));
    for i := 0 to masterCount - 1 do
      sl.Add('  [' + IntToStr(i) + '] ' + GetFileName(FileByIndex(i)));

    sl.Add('SUCCESS');
    sl.SaveToFile(logPath);
  finally
    sl.Free;
  end;

  AddMessage('House Rules smoke test wrote: ' + logPath);

  // Sentinel file — the PowerShell wrapper polls for this to know when to
  // post WM_CLOSE. Must be the LAST write the script performs.
  sl := TStringList.Create;
  try
    sl.Add('done');
    sl.SaveToFile(ProgramPath + 'HR_LastScriptDone.txt');
  finally
    sl.Free;
  end;

  Result := 0;
end;

function Process(e: IInterface): integer;
begin
  // Nothing to do — we did all our work in Initialize.
  // Returning non-zero here would abort; 0 = continue to next element.
  Result := 0;
end;

function Finalize: integer;
begin
  Result := 0;
end;

end.
