export type ThinkingPairEntry={number:string;chapter:number;path:string;study_date:string;published_date:string;topic_en:string};
export type ThinkingTopic={number:string;chapter:number;topic_en:string};
export type ThinkingBlockedNext={number:string;chapter:number;status:string;reason:string};
export type ThinkingPlan={approvedDate:string;edition:number;pagesPerTopic:number;targetTopicCount:number;complete:boolean;verifiedPrefix:(ThinkingTopic&{verification:string;evidence:{source:string;path:string}[]})[];sources:{id:string;url:string;revision?:string}[];blockedNext:ThinkingBlockedNext|null;unresolved:unknown[];unverifiedNumberedTopicCount:number;historicalBaseline:{number:string;study_date:string;published_date:string}[]};
export function koreanDate(now?:Date):string;
export function validateThinkingPairs(catalog:{episodes:ThinkingPairEntry[];current:ThinkingTopic&{study_date:string;published_date:string};next:ThinkingTopic|ThinkingBlockedNext|null},policy:{publicationUnit:string;pagesPerTopic:number;execution:string;scheduleEnabled:boolean;cadence:string;curriculumManifest:string},plan:ThinkingPlan,asOfKstDate?:string):{topics:number;pages:number};
