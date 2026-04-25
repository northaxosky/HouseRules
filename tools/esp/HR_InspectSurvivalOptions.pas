{
  House Rules -- inspect how SurvivalOptions patches HC_ReduceCarryWeightAbility.

  Invoke from FO4Edit:
    1. Load Fallout4.esm and SurvivalOptions.esp.
    2. Right-click a loaded file > Apply Script > HR_InspectSurvivalOptions.

  (Requires SurvivalOptions.esp deployed into the Data folder.)
  Dumps the full record structure to HR_InspectOut.txt.
}
unit UserScript;

var
  out: TStringList;

procedure DumpElement(e: IInterface; depth: integer);
var
  i, n: integer;
  child: IInterface;
  path, val, line: string;
begin
  if not Assigned(e) then Exit;
  path := Path(e);
  val := GetEditValue(e);
  line := StringOfChar(' ', depth * 2) + path;
  if val <> '' then line := line + ' = ' + val;
  out.Add(line);

  n := ElementCount(e);
  for i := 0 to n - 1 do begin
    child := ElementByIndex(e, i);
    DumpElement(child, depth + 1);
  end;
end;

function Initialize: integer;
var
  i, j: integer;
  f, grp, rec: IInterface;
  edid: string;
begin
  out := TStringList.Create;
  try
    out.Add('=== HR_InspectSurvivalOptions ===');
    out.Add('Scanning ' + IntToStr(FileCount) + ' files for SurvivalOptions...');

    for i := 0 to FileCount - 1 do begin
      f := FileByIndex(i);
      if Pos('SurvivalOptions', GetFileName(f)) > 0 then begin
        out.Add('');
        out.Add('>>> File: ' + GetFileName(f));
        grp := GroupBySignature(f, 'SPEL');
        if Assigned(grp) then begin
          for j := 0 to ElementCount(grp) - 1 do begin
            rec := ElementByIndex(grp, j);
            edid := GetElementEditValues(rec, 'EDID');
            if Pos('HC_Reduce', edid) > 0 then begin
              out.Add('');
              out.Add('---- SPEL: ' + edid + ' (' + IntToHex(GetLoadOrderFormID(rec), 8) + ') ----');
              DumpElement(rec, 0);
            end;
          end;
        end else begin
          out.Add('  (no SPEL group)');
        end;
      end;
    end;

    out.SaveToFile(ProgramPath + 'HR_InspectOut.txt');
    AddMessage('Inspector wrote: ' + ProgramPath + 'HR_InspectOut.txt');
  finally
    out.Free;
  end;

  // sentinel
  out := TStringList.Create;
  out.Add('done');
  out.SaveToFile(ProgramPath + 'HR_LastScriptDone.txt');
  out.Free;

  Result := 0;
end;

function Process(e: IInterface): integer; begin Result := 0; end;
function Finalize: integer; begin Result := 0; end;

end.
