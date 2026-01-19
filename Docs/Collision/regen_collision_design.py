import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parent
CSV_CHANNELS = ROOT / "Trace Channels-Collision Channel.csv"
CSV_PROFILES = ROOT / "Trace Channels-Collision Profile.csv"
CSV_REUSE = ROOT / "Trace Channels-Profile Reuse.csv"
OUT_MD = ROOT / "CollisionDesign.md"


def _read_csv_rows(path: Path) -> list[list[str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        return [[c.strip() for c in row] for row in csv.reader(f)]


def parse_channels() -> tuple[list[dict], list[dict]]:
    """
    Returns:
      trace_channels: [{name, desc, ignore, overlap, block}]
      object_channels: [{name, desc}]
    """
    rows = _read_csv_rows(CSV_CHANNELS)
    trace_channels: list[dict] = []
    object_channels: list[dict] = []

    section = None
    for row in rows:
        if not any(row):
            continue
        first = row[0] if len(row) > 0 else ""
        if first in ("Trace Channels", "Object Channels"):
            section = first
        if section == "Trace Channels":
            # Expected: section,name,desc,ignore,overlap,block,....
            name = row[1] if len(row) > 1 else ""
            desc = row[2] if len(row) > 2 else ""
            ignore = row[3] if len(row) > 3 else ""
            overlap = row[4] if len(row) > 4 else ""
            block = row[5] if len(row) > 5 else ""
            if name:
                trace_channels.append(
                    {"name": name, "desc": desc, "ignore": ignore, "overlap": overlap, "block": block}
                )
        elif section == "Object Channels":
            name = row[1] if len(row) > 1 else ""
            desc = row[2] if len(row) > 2 else ""
            if name:
                object_channels.append({"name": name, "desc": desc})

    return trace_channels, object_channels


def parse_profiles() -> tuple[list[str], dict[str, dict]]:
    """
    Returns:
      profile_names: in CSV order
      profiles: {profile_name: {"Collision Enabled": str, "Object Type": str, "responses": {channel: response}}}
    """
    rows = _read_csv_rows(CSV_PROFILES)
    if not rows:
        raise RuntimeError(f"Empty CSV: {CSV_PROFILES}")

    header = rows[0]
    profile_names = [h.strip() for h in header[1:] if h.strip()]
    profiles: dict[str, dict] = {
        pn: {"Collision Enabled": "", "Object Type": "", "responses": {}} for pn in profile_names
    }

    for row in rows[1:]:
        if not any(row):
            continue
        key = row[0].strip()
        if not key:
            continue
        vals = row[1 : 1 + len(profile_names)]
        if key in ("Collision Enabled", "Object Type"):
            for pn, v in zip(profile_names, vals):
                profiles[pn][key] = v.strip()
        else:
            # channel row
            for pn, v in zip(profile_names, vals):
                profiles[pn]["responses"][key] = v.strip()

    return profile_names, profiles


def parse_reuse() -> list[tuple[str, str]]:
    rows = _read_csv_rows(CSV_REUSE)
    out: list[tuple[str, str]] = []
    for row in rows:
        if not any(row):
            continue
        if row[0].strip() == "Profile复用":
            continue
        if len(row) >= 2 and row[0].strip() and row[1].strip():
            out.append((row[0].strip(), row[1].strip()))
    return out


def md_escape_cell(s: str) -> str:
    return s.replace("|", "\\|").strip()


def main() -> None:
    trace_channels, object_channels = parse_channels()
    profile_names, profiles = parse_profiles()
    reuse_rows = parse_reuse()

    # Keep channel response order as it appears in the profiles CSV.
    # (By reading the CSV top-to-bottom, insertion order is preserved in dicts in modern Python.)
    # We use the first profile as a template for ordering.
    first_profile = profiles[profile_names[0]] if profile_names else {"responses": {}}
    response_order = list(first_profile["responses"].keys())

    lines: list[str] = []
    # Header / intro
    lines.append("# \u78b0\u649e\u8bbe\u8ba1\uff08\u4ee5 CSV \u4e3a\u51c6\uff09")
    lines.append("")
    lines.append(
        "\u672c\u6587\u7528\u4e8e\u5728 UE \u91cc\u914d\u7f6e Collision Channel / Collision Profile\uff0c"
        "\u5e76\u4f5c\u4e3a\u540e\u7eed\u4ee3\u7801\u6539\u9020\u7684\u4f9d\u636e\u3002"
    )
    lines.append(
        "\u6240\u6709\u8868\u683c\u5185\u5bb9\u4ee5 `Docs/Collision/` \u76ee\u5f55\u4e0b 3 \u4e2a CSV "
        "\u4e3a\u201c\u552f\u4e00\u771f\u76f8\u201d\uff0c\u4e0d\u8981\u5728\u6587\u6863\u91cc\u624b\u6539\u8868\u683c\u800c\u4e0d\u6539 CSV\uff1a"
    )
    lines.append(f"- `{CSV_CHANNELS.name}`\uff1aChannel \u5b9a\u4e49\u4e0e\u8bed\u4e49\uff08Ignore/Overlap/Block \u7684\u542b\u4e49\uff09")
    lines.append(f"- `{CSV_PROFILES.name}`\uff1a\u5404 Collision Profile \u7684\u54cd\u5e94\u77e9\u9635")
    lines.append(f"- `{CSV_REUSE.name}`\uff1a\u540c\u4e00\u5b9e\u4f53\u5728\u4e0d\u540c\u72b6\u6001\u4e0b\u590d\u7528\u54ea\u4e2a Profile")
    lines.append("")

    # Principles
    lines.append("## \u8bbe\u8ba1\u539f\u5219")
    lines.append("- \u8fd0\u884c\u65f6\u5207\u6362\u201c\u72b6\u6001\u201d\u53ea\u5207\u6362 Collision Profile\uff1b\u5c3d\u91cf\u907f\u514d\u9010\u4e2a ResponseToChannel \u624b\u52a8\u6539\u3002")
    lines.append("- \u6293\u53d6/\u6295\u5c04\u7269\u68c0\u6d4b\u90fd\u8d70\u56fa\u5b9a\u7684 Trace Channel\uff08`Trace_Grab` / `Trace_Projectile`\uff09\u3002")
    lines.append("- \u4e3a\u201c\u73a9\u5bb6\u624b\u201d\u5355\u72ec\u5b9a\u4e49 Object Channel\uff08`Obj_PlayerHand`\uff09\uff0c\u7528\u4e8e\u80cc\u5305/\u5f13\u5f26\u7b49\u4ea4\u4e92\u533a\u57df\u7684\u68c0\u6d4b\uff0c\u907f\u514d\u7528 Tag \u5206\u652f\u3002")
    lines.append("")

    # Channels
    lines.append(f"## Channel\uff08\u6765\u81ea {CSV_CHANNELS.name}\uff09")
    lines.append("")
    lines.append("### Trace Channels")
    lines.append("| Name | Description | Ignore \u8bed\u4e49 | Overlap \u8bed\u4e49 | Block \u8bed\u4e49 |")
    lines.append("|---|---|---|---|---|")
    for ch in trace_channels:
        lines.append(
            f"| {md_escape_cell(ch['name'])} | {md_escape_cell(ch['desc'])} | "
            f"{md_escape_cell(ch['ignore'])} | {md_escape_cell(ch['overlap'])} | {md_escape_cell(ch['block'])} |"
        )
    lines.append("")
    lines.append("### Object Channels")
    lines.append("| Name | Description |")
    lines.append("|---|---|")
    for ch in object_channels:
        lines.append(f"| {md_escape_cell(ch['name'])} | {md_escape_cell(ch['desc'])} |")
    lines.append("")

    # Profiles
    lines.append(f"## Collision Profile\uff08\u6765\u81ea {CSV_PROFILES.name}\uff09")
    lines.append("")
    lines.append("\u8bf4\u660e\uff1a\u4e0b\u9762\u6bcf\u4e2a Profile \u90fd\u5217\u51fa `Collision Enabled`\u3001`Object Type`\uff0c\u4ee5\u53ca\u5bf9\u5404 Channel \u7684 Response\u3002")
    lines.append("")

    for pn in profile_names:
        p = profiles[pn]
        lines.append(f"### {pn}")
        lines.append(f"- Collision Enabled: `{p['Collision Enabled']}`")
        lines.append(f"- Object Type: `{p['Object Type']}`")
        lines.append("")
        lines.append("| Channel | Response |")
        lines.append("|---|---|")
        for ch_name in response_order:
            resp = p["responses"].get(ch_name, "")
            lines.append(f"| {md_escape_cell(ch_name)} | {md_escape_cell(resp)} |")
        lines.append("")

    # Reuse
    lines.append(f"## Profile \u590d\u7528\uff08\u6765\u81ea {CSV_REUSE.name}\uff09")
    lines.append("")
    lines.append("| \u5b9e\u4f53/\u72b6\u6001 | \u4f7f\u7528\u7684 Profile |")
    lines.append("|---|---|")
    for state, prof in reuse_rows:
        lines.append(f"| {md_escape_cell(state)} | {md_escape_cell(prof)} |")
    lines.append("")

    # Notes (hand-maintained, but still derived from decisions already made)
    lines.append("## \u5b9e\u73b0\u6ce8\u610f\u4e8b\u9879\uff08\u540e\u7eed\u6539\u4ee3\u7801\u65f6\u9075\u5b88\uff09")
    lines.append("- VR \u6293\u53d6\u5019\u9009\u6536\u96c6\uff1a\u4e0d\u8981\u7528 Sweep\uff08\u4f1a\u88ab\u7b2c\u4e00\u4e2a Block \u7ec8\u6b62\u3001\u62ff\u4e0d\u5230\u591a\u4e2a\u5019\u9009\uff09\uff1b\u7528 `GetWorld()->OverlapMultiByChannel(Trace_Grab)` \u6536\u96c6\uff0c\u518d\u6309 `IGrabbable::CanBeGrabbedBy` / \u8ddd\u79bb\u7b49\u89c4\u5219\u7b5b\u9009\u6700\u8fd1\u76ee\u6807\u3002")
    lines.append("- PC \u6293\u53d6\uff1a\u5982\u679c\u7ee7\u7eed\u7528 `LineTraceByChannel(Trace_Grab)`\uff0c\u76ee\u6807\u9700\u8981\u5bf9 `Trace_Grab` \u4e3a `Block` \u624d\u80fd\u547d\u4e2d\uff08\u672c CSV \u7684\u6293\u53d6\u7269 Profile \u5df2\u914d\u7f6e\u4e3a `Block`\uff09\u3002")
    lines.append("- \u8bed\u4e49\u5907\u6ce8\uff1aChannel \u8bed\u4e49\u8868\u91cc `Trace_Grab` \u7684 Overlap \u542b\u4e49\u5199\u4e86\u201c\u53ef\u88ab\u6293\u53d6\u201d\uff0c\u4f46\u5f53\u524d Profile \u77e9\u9635\u4e2d\u591a\u4e2a\u201c\u53ef\u6293\u53d6\u7269\u4f53\u201d\u5bf9 `Trace_Grab` \u7528\u7684\u662f `Block`\uff1b\u8fd9\u662f\u4e3a\u4e86\u517c\u5bb9 PC \u7684 LineTrace \u547d\u4e2d\u7b56\u7565\uff0cVR \u4ecd\u7136\u7528 OverlapMulti \u62ff\u5019\u9009\u3002")
    lines.append("- \u80cc\u5305\u903b\u8f91\u8fc1\u79fb\uff08TODO\uff09\uff1a\u76ee\u524d\u80cc\u5305\u68c0\u6d4b\u7531\u201c\u624b\u201d\u6765\u505a\uff1b\u82e5\u7edf\u4e00\u7528 `Obj_PlayerHand` \u505a\u68c0\u6d4b\uff0c\u5efa\u8bae\u628a\u80cc\u5305 enter/exit \u7684\u5224\u5b9a\u903b\u8f91\u8fc1\u79fb\u5230 `ABaseVRPlayer`\uff08\u7531\u73a9\u5bb6\u7edf\u4e00\u5904\u7406\u5de6\u53f3\u624b\u7684 overlap\uff09\uff0c\u907f\u514d\u5230\u5904\u67e5 Tag\u3002")
    lines.append("")

    OUT_MD.write_text("\n".join(lines), encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
